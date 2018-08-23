/*
** EZIoT - Random Generator Class
** (TrueRandom - A true random number generator for Arduino.)
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
#ifndef _EZ_RANDOM_H
#define _EZ_RANDOM_H
#include <Arduino.h>
#include <inttypes.h>

#ifndef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR
#endif

namespace EZ
{
    class RANDOM
    {
    public:
        ICACHE_FLASH_ATTR RANDOM();
        ICACHE_FLASH_ATTR int rand();
        ICACHE_FLASH_ATTR long random();
        ICACHE_FLASH_ATTR long random(long howBig);
        ICACHE_FLASH_ATTR long random(long howsmall, long how);
        ICACHE_FLASH_ATTR int randomBit(void);
        ICACHE_FLASH_ATTR char randomByte(void);
        ICACHE_FLASH_ATTR void memfill(char* location, int size);
        bool useRNG;

    private:
        unsigned long lastYield;
        ICACHE_FLASH_ATTR int randomBitRaw(void);
        ICACHE_FLASH_ATTR int randomBitRaw2(void);
    };

} // namespace EZ
#endif // _EZIRANDOM_H

/******************************************************************************/