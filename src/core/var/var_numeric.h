/*
** EZIoT - String Enumeration Variable Class
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
#if !defined(_EZI_VAR_NUMERIC_H)
#define _EZI_VAR_NUMERIC_H
#include "../ez_variable.h"

namespace EZ
{
    namespace VAR
    {
        template<class _T> class NUMERIC : public VARIABLE
        {
            friend class IOT;

        public:
            virtual ~NUMERIC() {}
            NUMERIC(const char* name, const char* type, bool evt, bool nvs, _T defVal, _T minVal, _T maxVal, _T stepVal)
                : VARIABLE(name, type, evt, nvs, sizeof(_T)), _minValue(minVal), _maxValue(maxVal), _stepValue(stepVal)
            {
                _activeValue = _defaultValue = defVal;
            }

            String defaultValue(void) { return _toString(_defaultValue); }

            int validate(String& value)
            {
                _T nv;

                _deString(value, nv);

                // Value must be in range?
                if (nv < _minValue || nv > _maxValue)
                    return EZ_SOAP_ERROR_OUT_OF_RANGE;

                // Value must be in step (if any)?
                if ((_stepValue != 0) && (nv % _stepValue) != 0)
                    return EZ_SOAP_ERROR_INVALID_VALUE;

                return EZ_SOAP_ERROR_NONE;
            }

            _T native(void) const { return _activeValue; }
            bool native(_T nv) { return value(String(nv)); }

        protected:
            _T _activeValue;
            _T _defaultValue;
            _T _minValue;
            _T _maxValue;
            _T _stepValue;

            String _allowedTags(void)
            {
                String tags("");

                if (_minValue != _maxValue)
                {
                    tags.concat(xmlTag("minimum", _toString(_minValue), true));
                    tags.concat(xmlTag("maximum", _toString(_maxValue), true));
                }

                if (_stepValue != 0)
                    tags.concat(xmlTag("step", _toString(_stepValue), true));

                return xmlTag("allowedValueRange", tags, false);
            }

            String _getValue(void) { return _toString(_activeValue); }

            bool _setValue(String& newValue)
            {
                _T nv;

                _deString(newValue, nv);

                // Value must be in range?
                if (_minValue != _maxValue)
                {
                    if (nv < _minValue)
                        nv = _minValue;
                    if (nv > _maxValue)
                        nv = _maxValue;
                }

                // Value must be in step (if any)?
                if ((_stepValue != 0) && (nv % _stepValue) != 0)
                    return false;

                // Changed? - set new value and advise
                if (_activeValue != nv)
                {
                    _activeValue = nv;
                    return true;
                }

                return false;
            }

        private:
            void _deString(String& val, char& nv) { nv = (char)val.toInt(); }
            void _deString(String& val, int8_t& nv) { nv = (int8_t)val.toInt(); }
            void _deString(String& val, uint8_t& nv) { nv = (uint8_t)val.toInt(); }
            void _deString(String& val, int16_t& nv) { nv = (int16_t)val.toInt(); }
            void _deString(String& val, uint16_t& nv) { nv = (uint16_t)val.toInt(); }
            void _deString(String& val, int32_t& nv) { nv = (int32_t)val.toInt(); }
            void _deString(String& val, uint32_t& nv) { nv = (uint32_t)val.toInt(); }
            void _deString(String& val, float& nv) { nv = val.toFloat(); }
            void _deString(String& val, double& nv) { nv = (double)val.toFloat(); }

            String _toString(char t) { return String(t); }
            String _toString(int8_t t) { return String(t); }
            String _toString(int16_t t) { return String(t); }
            String _toString(int32_t t) { return String(t); }
            String _toString(uint8_t t) { return String(t); }
            String _toString(uint16_t t) { return String(t); }
            String _toString(uint32_t t) { return String(t); }
            String _toString(float t) { return String(t, 2); }
            String _toString(double t) { return String(t, 4); }

            // TODO: Need to code these for 64 bit ints!
            void _deString(String& val, int64_t& nv) {}
            void _deString(String& val, uint64_t& nv) {}
            String _toString(int64_t t) { return "TODO"; }
            String _toString(uint64_t t) { return "TODO"; }
        };

        /*
         ** UI1 - Unsigned 1 Byte int. Same format as int without leading sign.
         */
        class UI1 : public NUMERIC<uint8_t>
        {
        public:
            UI1(const char* name, bool events, bool nvm, uint8_t defVal = 0, uint8_t minVal = 0,
                uint8_t maxVal = UCHAR_MAX, uint8_t stepVal = 0)
                : NUMERIC(name, "ui1", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_u8(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u8(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** UI2 - Unsigned 2 Byte int. Same format as int without leading sign.
        */
        class UI2 : public NUMERIC<uint16_t>
        {
        public:
            UI2(const char* name, bool events, bool nvm, uint16_t defVal = 0, uint16_t minVal = 0,
                uint16_t maxVal = USHRT_MAX, uint16_t stepVal = 0)
                : NUMERIC(name, "ui2", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_u16(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u16(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** UI4 - Unsigned 4 Byte int. Same format as int without leading sign.
        */
        class UI4 : public NUMERIC<uint32_t>
        {
        public:
            UI4(const char* name, bool events, bool nvm, uint32_t defVal = 0, uint32_t minVal = 0,
                uint32_t maxVal = UINT_MAX, uint32_t stepVal = 0)
                : NUMERIC(name, "ui4", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_u32(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u32(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** UI8 - Unsigned 8 Byte int. Same format as int without leading sign.
        */
        class UI8 : public NUMERIC<uint64_t>
        {
        public:
            UI8(const char* name, bool events, bool nvm, uint64_t defVal = 0, uint64_t minVal = 0,
                uint64_t maxVal = ULONG_MAX, uint64_t stepVal = 0)
                : NUMERIC(name, "ui8", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_u64(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u64(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
         ** I1 - 1 Byte int. Same format as int.
         */
        class I1 : public NUMERIC<int8_t>
        {
        public:
            I1(const char* name, bool events, bool nvm, int8_t defVal = 0, int8_t minVal = SCHAR_MIN,
               int8_t maxVal = SCHAR_MAX, int8_t stepVal = 0)
                : NUMERIC(name, "i1", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_i8(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i8(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** I2 - 2 Byte int. Same format as int.
        */
        class I2 : public NUMERIC<int16_t>
        {
        public:
            I2(const char* name, bool events, bool nvm, int16_t defVal = 0, int16_t minVal = SHRT_MIN,
               int16_t maxVal = SHRT_MAX, int16_t stepVal = 0)
                : NUMERIC(name, "i2", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_i16(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i16(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** I4 - 4 Byte int. Same format as int. shall be between
        ** -2147483648 and 2147483647
        */
        class I4 : public NUMERIC<int32_t>
        {
        public:
            I4(const char* name, bool events, bool nvm, int32_t defVal = 0, int32_t minVal = INT_MAX,
               int32_t maxVal = INT_MAX, int32_t stepVal = 0)
                : NUMERIC(name, "i4", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_i32(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i32(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
        ** I8 - 8 Byte int. Same format as int. shall be between
        ** -9,223,372,036,854,775,808 and 9,223,372,036,854,775,807
        */
        class I8 : public NUMERIC<int64_t>
        {
        public:
            I8(const char* name, bool events, bool nvm, int64_t defVal = 0, int64_t minVal = LONG_MAX,
               int64_t maxVal = LONG_MAX, int64_t stepVal = 0)
                : NUMERIC(name, "i8", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_i64(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i64(nvsHandle, name().c_str(), _activeValue); }
        };

        /*
         ** INT - 4 Byte int. -2147483648 and 2147483647
         */
        class INT : public NUMERIC<int32_t>
        {
        public:
            INT(const char* name, bool events, bool nvm, int32_t defVal = 0, int32_t minVal = INT_MAX,
                int32_t maxVal = INT_MAX, int32_t stepVal = 0)
                : NUMERIC(name, "int", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_i32(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i32(nvsHandle, name().c_str(), _activeValue); }
        };

        // TODO: r4, r8, number(=r8), fixed.14.4 and float

    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_NUMERIC_H
