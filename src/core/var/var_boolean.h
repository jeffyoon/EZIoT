/*
** EZIoT - Boolean Variable Class
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
#if !defined(_EZI_VAR_BOOLEAN_H)
#define _EZI_VAR_BOOLEAN_H
#include "../ez_variable.h"

namespace EZ
{
    namespace VAR
    {
        class BOOLEAN : public VARIABLE
        {
            friend class IOT;

        public:
            ~BOOLEAN() {}
            BOOLEAN(const char* name, bool evt, bool nvs, bool defVal = false)
                : VARIABLE(name, "boolean", evt, nvs, sizeof(bool)), _activeValue(defVal), _defaultValue(defVal)
            {
            }

            int validate(String& value)
            {
                return _checkValue(value) == "" ? EZ_SOAP_ERROR_INVALID_VALUE : EZ_SOAP_ERROR_NONE;
            }

            String defaultValue(void) { return _defaultValue ? "1" : "0"; }

            bool native(void) const { return _activeValue; }
            bool native(bool nv) { return value(String(nv)); }

        protected:
            bool _activeValue;
            bool _defaultValue;

            esp_err_t _loadValue(uint32_t nvsHandle)
            {
                return nvs_get_u8(nvsHandle, name().c_str(), (uint8_t*)&_activeValue);
            }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_u8(nvsHandle, name().c_str(), _activeValue); }

            String _getValue(void) { return _activeValue ? "1" : "0"; }

            bool _setValue(String& newValue)
            {
                if ((newValue = _checkValue(newValue)) != "")
                {
                    bool nv = newValue == "1" ? true : false;

                    if (_activeValue != nv)
                    {
                        _activeValue = nv;
                        return true;
                    }
                }
                return false;
            }

        private:
            String _checkValue(String& value)
            {
                if (value == "1" || value.equalsIgnoreCase("true") || value.equalsIgnoreCase("on"))
                {
                    return "1";
                }
                else if (value != "0" && !value.equalsIgnoreCase("false") && !value.equalsIgnoreCase("off"))
                    return "";
                return "0";
            }
        };
    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_BOOLEAN_H
