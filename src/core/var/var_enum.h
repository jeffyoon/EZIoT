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
#if !defined(_EZI_VAR_ENUM_H)
#define _EZI_VAR_ENUM_H
#include "../ez_variable.h"

namespace EZ
{
    namespace VAR
    {
        class ENUM : public VARIABLE
        {
            friend class IOT;

        public:
            ~ENUM() {}
            ENUM(const char* name, bool evt, bool nvs, uint16_t defVal = 0, const char** list = nullptr)
                : VARIABLE(name, "string", evt, nvs, sizeof(uint16_t)), _activeValue(defVal), _defaultValue(defVal),
                  _listMax(0), _listVal(list)
            {
                if (list)
                {
                    for (_listMax = 0; list[_listMax] != nullptr; _listMax++)
                        ;
                    if (defVal < 0 || defVal > _listMax)
                        _activeValue = _defaultValue = defVal;
                }
                else
                    _activeValue = _defaultValue = 0;
            }

            int validate(String& value)
            {
                if (value.length() > 50)
                    return EZ_SOAP_ERROR_STRING_LENGTH;

                for (int nv = 0; nv < _listMax; nv++)
                {
                    if (value.equalsIgnoreCase(_listVal[nv]))
                        return EZ_SOAP_ERROR_NONE;
                }

                return EZ_SOAP_ERROR_OUT_OF_RANGE;
            }

            String defaultValue(void)
            {
                if (_listMax > 0 && _listVal != nullptr && _defaultValue < _listMax)
                    return String(_listVal[_defaultValue]);
                return "";
            }

        protected:
            esp_err_t _loadValue(uint32_t nvsHandle) { return nvs_get_u16(nvsHandle, name().c_str(), &_activeValue); }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u16(nvsHandle, name().c_str(), _activeValue); }

            String _allowedTags(void)
            {
                String tags("");

                for (int v = 0; v < _listMax; v++)
                {
                    tags.concat(xmlTag("allowedValue", _listVal[v], true));
                }

                return xmlTag("allowedValueList", tags, false);
            }

            String _getValue(void)
            {
                if (_listMax > 0 && _listVal != nullptr && _activeValue < _listMax)
                    return String(_listVal[_activeValue]);
                return defaultValue();
            }

            bool _setValue(String& newValue)
            {
                uint16_t nv;

                for (nv = 0; nv < _listMax; nv++)
                {
                    if (newValue.equalsIgnoreCase(_listVal[nv]))
                    {
                        if (_activeValue != nv)
                        {
                            _activeValue = nv;
                            return true;
                        }
                        break;
                    }
                }
                return false;
            }

        private:
            uint16_t _activeValue;
            uint16_t _defaultValue;
            uint16_t _listMax;
            const char** _listVal;
        };
    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_ENUM_H
