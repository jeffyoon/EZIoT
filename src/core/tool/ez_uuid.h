/*
** EZIoT - UUID Utility Class
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZ_UUID_H
#define _EZ_UUID_H
#include "ez_random.h"

/*
** Defintions and Equates
*/
#define EZ_UUID_LENGTH 36
#define EZ_UUID_MAC_INDEX 24
#define EZ_UUID_TIME_OFFSET_HIGH 0x01B21DD2
#define EZ_UUID_TIME_OFFSET_LOW 0x13814000

namespace EZ
{
    class UUID : protected RANDOM
    {
    protected:
        union _uuid {
            uint8_t b[16];
            struct
            {
                uint64_t ll1;
                uint64_t ll2;
            };
            struct
            {
                uint32_t time_low;
                uint16_t time_mid;
                uint16_t time_hi_and_version;
                uint16_t clock_seq;
                uint8_t node[6];
            };
        } _uuid;

        // void _mac(uint8_t* macLocation) { _memfill((char*)macLocation, 6); }

        void _memfill(char* location, int size)
        {
            useRNG = true;
            for (; size--;)
                *location++ = randomByte();
        }

    public:
        virtual ~UUID() {}
        UUID(bool v4 = false)
        {
            _uuid.ll1 = _uuid.ll2 = 0;
            if (v4)
                makeV4();
        }

        UUID(String uuid) : UUID() { (void)fromString(uuid); }
        UUID(const char* uuid) : UUID() { (void)fromString(uuid); }
        uint8_t operator[](int index) const { return _uuid.b[index]; }
        uint8_t& operator[](int index) { return _uuid.b[index]; }
        bool operator==(const UUID& uuid) const { return (_uuid.ll1 == uuid._uuid.ll1 && _uuid.ll2 == uuid._uuid.ll2); }
        bool operator==(bool empty) const { return (_uuid.ll1 != 0 && _uuid.ll2 != 0); }
        bool operator==(const char* uuid) const { return (String(uuid) == toString()); }
        bool operator==(String uuid) const { return (uuid == toString()); }

        size_t size(void) { return sizeof(_uuid); }
        uint8_t* raw_address() { return _uuid.b; }

        bool fromString(const String& uuid) { return fromString(uuid.c_str()); }
        bool fromString(const char* uuid)
        {
            // uuid:59b85c70-9236-4d3c-9546-d12fe6e42e0e
            // 59b85c70-9236-4d3c-9546-d12fe6e42e0e

            if ((!uuid) || strlen(uuid) < EZ_UUID_LENGTH)
                return false;

            const char* cp;
            char buf[3];
            int i = 0;

            if ((i = strncasecmp(uuid, "uuid:", 5)) == 0)
                uuid += 5;

            for (i = 0, cp = uuid; i <= EZ_UUID_LENGTH; i++, cp++)
            {
                if ((i == 8) || (i == 13) || (i == 18) || (i == 23))
                {
                    if (*cp == '-')
                        continue;
                    else
                        return false;
                }
                if (i == 36)
                {
                    if (*cp == 0)
                        continue;
                }
                if (!isxdigit((int)*cp))
                    return false;
            }

            cp = uuid;
            buf[2] = 0;

            for (i = 0; i < 16; i++)
            {
                if (*cp == '-')
                    cp++;
                buf[0] = *cp++;
                buf[1] = *cp++;
                _uuid.b[i] = strtoul(buf, NULL, 16);
            }

            return true;
        }

        String toString(bool prefix = false) const
        {
            String string = prefix ? "uuid:" : "";
            int i;

            for (i = 0; i < 16; i++)
            {
                if (i == 4)
                    string += "-";
                if (i == 6)
                    string += "-";
                if (i == 8)
                    string += "-";
                if (i == 10)
                    string += "-";
                int topDigit = _uuid.b[i] >> 4;
                int bottomDigit = _uuid.b[i] & 0x0f;
                // High hex digit
                string += "0123456789abcdef"[topDigit];
                // Low hex digit
                string += "0123456789abcdef"[bottomDigit];
            }

            return string;
        }

        bool isZero(void)
        {
            return (_uuid.ll1 == 0 && _uuid.ll2 == 0);
        }

        void makeZero(void)
        {
            _uuid.ll1 = _uuid.ll2 = 0;
        }

        void makeV4(void)
        {
            // Generate a Version 4 UUID according to RFC4122
            _memfill((char*)_uuid.b, 16);

            // Although the UUID contains 128 bits, only 122 of those are random.
            // The other 6 bits are fixed, to indicate a version number.
            _uuid.b[6] = 0x40 | (0x0F & _uuid.b[6]);
            _uuid.b[8] = 0x80 | (0x3F & _uuid.b[8]);
        }
    };
} // namespace EZ
#endif // _EZ_UUID_H

/******************************************************************************/