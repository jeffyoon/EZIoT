/*
** EZIoT - Service UUID "Status" Variable Class
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
#if !defined(_EZ_VAR_UUID_H)
#define _EZ_VAR_UUID_H
#include "../ez_variable.h"
#include "../tool/ez_uuid.h"

namespace EZ
{
    namespace VAR
    {
        class UUID : public VARIABLE
        {
            friend class IOT;

        public:
            ~UUID() {}

            UUID(const char* name, bool evt, bool nvs, String uu) : UUID(name, evt, nvs, false)
            {
                _activeValue.fromString(uu);
            }

            UUID(const char* name = "uuid:", bool evt = false, bool nvs = false, bool v4 = true)
                : VARIABLE(name, "uuid", evt, nvs, _activeValue.size())
            {
                if (v4)
                    _activeValue.makeV4();
            }

            int validate(String& value)
            {
                EZ::UUID tmp(value);

                return tmp == false ? EZ_SOAP_ERROR_INVALID_VALUE : EZ_SOAP_ERROR_NONE;
            }

            bool isZero(void)
            {
                if (_nvs && !_nvsLoaded && _activeValue.isZero())
                {
                    if (!_nvsLoad())
                        _activeValue.makeZero();
                }
                return _activeValue.isZero();
            }

            String makeV4(void)
            {
                ESP_LOGV(iotTag, "\n** MAKEV4() %d**\n", _nvs);

                _activeValue.makeV4();
                if (_nvs)
                    (void)_nvsSave();
                return _activeValue.toString();
            }

        protected:
            esp_err_t _loadValue(uint32_t nvsHandle)
            {
                size_t len = 0;

                esp_err_t err = nvs_get_blob(nvsHandle, name().c_str(), NULL, &len);

                if (!err)
                {
                    if (len == _activeValue.size())
                        return nvs_get_blob(nvsHandle, name().c_str(), _activeValue.raw_address(), &len);
                    err = ESP_ERR_NVS_TYPE_MISMATCH;
                }

                return err;
            }

            esp_err_t _saveValue(uint32_t nvsHandle)
            {
                return nvs_set_blob(nvsHandle, name().c_str(), _activeValue.raw_address(), _activeValue.size());
            }

            String _getValue(void) { return _activeValue.toString(); }

            bool _setValue(String& newValue)
            {
                if (!(_activeValue == newValue))
                {
                    _activeValue.fromString(newValue);
                    return true;
                }

                return false;
            }

        private:
            EZ::UUID _activeValue;
        };
    } // namespace VAR
} // namespace EZ
#endif //_EZ_VAR_UUID_H

/******************************************************************************/