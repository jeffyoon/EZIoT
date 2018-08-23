/*
** EZIoT - Random Generator Class (TrueRandom - A true random number generator
** for Arduino.)
**
** This is variant of original work originally implemented as:
** https://code.google.com/archive/p/tinkerit/
** https://github.com/Cathedrow/TrueRandom
** Copyright (c) 2010 Peter Knight, Tinker.it! All rights reserved.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#include "ez_random.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "crypto/random.h"
#define EZ_RANDOM_USE_RNG false
#define DR_REG_RNG_BASE 0x3ff75144
#else
#define EZ_RANDOM_USE_RNG true
#endif

using namespace EZ;

ICACHE_FLASH_ATTR RANDOM::RANDOM()
{
    useRNG = EZ_RANDOM_USE_RNG;
    lastYield = 0;
}

ICACHE_FLASH_ATTR int RANDOM::randomBitRaw(void)
{
    // Needed to keep wifi stack running smoothly
    // And to avoid wdt reset
    if (lastYield == 0 || millis() - lastYield >= 50)
    {
        yield();
        lastYield = millis();
    }
#if defined(ARDUINO_ARCH_ESP32)
    uint32_t bit = useRNG ? READ_PERI_REG(DR_REG_RNG_BASE) : analogRead(A0); // random();
#else
    uint8_t bit = useRNG ? (int)RANDOM_REG32 // using the onboard hardware random
                                             // number generator (esp8266_peri.h)
                         : analogRead(A0);   // using A0 / TOUT
#endif
    return bit & 1;
}

ICACHE_FLASH_ATTR int RANDOM::randomBitRaw2(void)
{
    // Software whiten bits using Von Neumann algorithm
    //
    // von Neumann, John (1951). "Various techniques used in connection
    // with random digits". National Bureau of Standards Applied Math Series
    // 12:36.
    //
    for (;;)
    {
        int a = randomBitRaw() | (randomBitRaw() << 1);
        if (a == 1)
            return 0; // 1 to 0 transition: log a zero bit
        if (a == 2)
            return 1; // 0 to 1 transition: log a one bit
                      // For other cases, try again.
    }
    return 0;
}

ICACHE_FLASH_ATTR int RANDOM::randomBit(void)
{
    // Software whiten bits using Von Neumann algorithm
    //
    // von Neumann, John (1951). "Various techniques used in connection
    // with random digits". National Bureau of Standards Applied Math Series
    // 12:36.
    //
    for (;;)
    {
        int a = randomBitRaw2() | (randomBitRaw2() << 1);
        if (a == 1)
            return 0; // 1 to 0 transition: log a zero bit
        if (a == 2)
            return 1; // 0 to 1 transition: log a one bit
                      // For other cases, try again.
    }
    return 0;
}

ICACHE_FLASH_ATTR char RANDOM::randomByte(void)
{
    char result = 0;
    uint8_t i;
    for (i = 8; i--;)
        result += result + randomBit();
    return result;
}

ICACHE_FLASH_ATTR int RANDOM::rand()
{
    int result = 0;
    uint8_t i;
    for (i = 15; i--;)
        result += result + randomBit();
    return result;
}

ICACHE_FLASH_ATTR long RANDOM::random()
{
    long result = 0;
    uint8_t i;
    for (i = 31; i--;)
        result += result + randomBit();
    return result;
}

ICACHE_FLASH_ATTR long RANDOM::random(long howBig)
{
    long randomValue;
    long topBit;
    long bitPosition;

    if (!howBig)
        return 0;
    randomValue = 0;
    if (howBig & (howBig - 1))
    {
        // Range is not a power of 2 - use slow method
        topBit = howBig - 1;
        topBit |= topBit >> 1;
        topBit |= topBit >> 2;
        topBit |= topBit >> 4;
        topBit |= topBit >> 8;
        topBit |= topBit >> 16;
        topBit = (topBit + 1) >> 1;

        bitPosition = topBit;
        do
        {
            // Generate the next bit of the result
            if (randomBit())
                randomValue |= bitPosition;

            // Check if bit
            if (randomValue >= howBig)
            {
                // Number is over the top limit - start again.
                randomValue = 0;
                bitPosition = topBit;
            }
            else
            {
                // Repeat for next bit
                bitPosition >>= 1;
            }
        } while (bitPosition);
    }
    else
    {
        // Special case, howBig is a power of 2
        bitPosition = howBig >> 1;
        while (bitPosition)
        {
            if (randomBit())
                randomValue |= bitPosition;
            bitPosition >>= 1;
        }
    }
    return randomValue;
}

ICACHE_FLASH_ATTR long RANDOM::random(long howSmall, long howBig)
{
    if (howSmall >= howBig)
        return howSmall;
    long diff = howBig - howSmall;
    return RANDOM::random(diff) + howSmall;
}
/******************************************************************************/