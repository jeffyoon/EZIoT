/*
** EZIoT - String Variable Class
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
#if !defined(_EZI_VAR_STRING_H)
#define _EZI_VAR_STRING_H
#include "../ez_variable.h"

namespace EZ
{
    namespace VAR
    {
        class STRING : public VARIABLE
        {
            friend class IOT;

        public:
            ~STRING() {}
            STRING(const char* name, bool evt, bool nvs, const char* defVal = "", size_t maxLen = 64)
                : VARIABLE(name, "string", evt, nvs, min(maxLen, 256L)), _activeValue(defVal), _defaultValue(defVal)
            {
            }

            int validate(String& value)
            {
                if (value.length() > _size)
                    return EZ_SOAP_ERROR_STRING_LENGTH;
                return EZ_SOAP_ERROR_NONE;
            }

            String defaultValue(void) { return _defaultValue; }

        protected:
            String _allowedTags(void) { return ""; }

            String _getValue(void) { return _activeValue; }

            bool _setValue(String& newValue)
            {
                if (_activeValue != newValue)
                {
                    _activeValue = newValue;
                    return true;
                }

                return false;
            }

            esp_err_t _loadValue(uint32_t nvsHandle)
            {
                size_t len = 0;
                esp_err_t err = nvs_get_str(nvsHandle, name().c_str(), NULL, &len);

                if (!err)
                {
                    if (len <= _size)
                    {
                        if (_activeValue.reserve(len))
                        {
                            char value[len];
                            len = _size;
                            err = nvs_get_str(nvsHandle, name().c_str(), value, &len);

                            if (!err)
                            {
                                _activeValue = value;
                            }

                            return err;
                        }
                    }

                    return ESP_ERR_NVS_NOT_ENOUGH_SPACE;
                }

                return err;
            }

            esp_err_t _saveValue(uint32_t nvsHandle)
            {
                return nvs_set_str(nvsHandle, name().c_str(), _activeValue.c_str());
            }

        private:
            String _activeValue;
            String _defaultValue;
        };
    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_STRING_H
