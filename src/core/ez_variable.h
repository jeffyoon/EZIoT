/*
** EZIoT - Variable Base Class
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
#if !defined(_EZ_VARIABLE_H)
#define _EZ_VARIABLE_H
#include "ez_activity.h"
#include "ez_common.h"

//        uint32_t _nvsHandle;

namespace EZ
{
    class VARIABLE : public ACTIVITY
    {
        friend class IOT;
        friend class SERVICE;

    public:
        virtual ~VARIABLE() {}
        VARIABLE(const char* name, const char* type, bool events, bool nvs, size_t size)
            : ACTIVITY(name, ACTIVITY::MODE::VARIABLE), _events(events), _nvs(nvs), _size(size), _type(type),
              _nvsLoaded(false)
        {
        }

        String type(void) { return _type; }
        bool nvs(void) { return _nvs; }
        void nvs(bool nvsSave) { _nvs = nvsSave; }
        bool value(const char* newValue) { return value(String(newValue)); }

        bool value(String newValue)
        {
            bool _hasChanged = false;

            if (!_takeServiceMutex())
                return false;

            if (_postCallback(SERVICE::CALLBACK::PRE_CHANGE, &newValue))
            {
                if ((_hasChanged = _setValue(newValue)))
                {
                    if (_postCallback(SERVICE::CALLBACK::POST_CHANGE))
                    {
                        // Save to NVS?
                        if (_nvs && homeService())
                            _nvsSave();

                        if (_events && homeService())
                            homeService()->registerEvent(this);
                    }
                }
            }

            _giveServiceMutex();

            // ESP_LOGV(iotTag, "%s - hasChanged: %d", name().c_str(), _hasChanged);

            return _hasChanged;
        }

        String value(void)
        {
            _takeServiceMutex();
            if (_nvs && !_nvsLoaded)
                (void)_nvsLoad();
            String value = _getValue();
            _giveServiceMutex();

            return value;
        }

        String upnpXML(bool valueTag = false, bool emptyTag = false)
        {
            if (valueTag)
            {
                // Encode Value?
                return xmlTag(name().c_str(), value(), emptyTag);
            }

            String xml("<stateVariable sendEvents=\"{e}\">\r\n");
            xml.replace("{e}", (_events ? "yes" : "no"));
            xml.concat(xmlTag("name", name(), true));
            xml.concat(xmlTag("dataType", type(), true));
            xml.concat(xmlTag("defaultValue", defaultValue(), false));
            xml.concat(_allowedTags());
            xml.concat("</stateVariable>\r\n");

            return xml;
        }

        bool upnpEventable(void) { return _events; }
        virtual String defaultValue(void) { return ""; }
        virtual int validate(String& val) = 0;

    protected:
        bool _events;
        bool _nvs;
        size_t _size;
        String _type;
        bool _nvsLoaded;

        bool _nvsErase(void)
        {
            if (!homeService())
                return false;

            uint32_t nvsHandle = homeService()->nvsHandle();
            esp_err_t err = nvs_erase_key(nvsHandle, name().c_str());

            if (err)
            {
                ESP_LOGV(iotTag, "NVS: Erase key failed: %s %s", name().c_str(), nvs_error(err));
                return false;
            }

            ESP_LOGV(iotTag, "NVS: Loaded OK: %s", name().c_str());

            return true;
        }

        bool _nvsLoad(void)
        {
            if (!homeService())
                return false;

            uint32_t nvsHandle;

            if ((nvsHandle = homeService()->nvsHandle()))
            {
                esp_err_t err = _loadValue(nvsHandle);

                if ((err) && err != ESP_ERR_NVS_NOT_FOUND)
                {
                    ESP_LOGV(iotTag, "NVS: Get failed: %s %s", name().c_str(), nvs_error(err));
                    return false;
                }
                else
                    _nvsLoaded = true;

                ESP_LOGV(iotTag, "NVS: Loaded OK: %s", name().c_str());

                return true;
            }
            return false;
        }

        bool _nvsSave(void)
        {
            if (!homeService())
                return false;

            uint32_t nvsHandle;

            if ((nvsHandle = homeService()->nvsHandle()))
            {
                esp_err_t err = _saveValue(nvsHandle);

                ESP_LOGV(iotTag, "NVS: %s %s", name().c_str(), nvs_error(err));

                if (err)
                {
                    ESP_LOGV(iotTag, "NVS: Set failed: %s %s", name().c_str(), nvs_error(err));
                    return false;
                }

                err = nvs_commit(nvsHandle);

                if (err)
                {
                    ESP_LOGV(iotTag, "NVS: Commit failed: %s %s", name().c_str(), nvs_error(err));
                    return false;
                }

                ESP_LOGV(iotTag, "NVS: Saved OK: %s", name().c_str());

                return true;
            }
            return false;
        }

        virtual esp_err_t _loadValue(uint32_t nvsHandle) = 0;
        virtual esp_err_t _saveValue(uint32_t nvsHandle) = 0;
        virtual bool _setValue(String& val) = 0;
        virtual String _getValue(void) = 0;
        virtual String _allowedTags(void) { return ""; }
    };
} // namespace EZ

// Let's have em all!
//
#include "var/var_bin64.h"
#include "var/var_binHex.h"
#include "var/var_boolean.h"
#include "var/var_char.h"
#include "var/var_enum.h"
#include "var/var_numeric.h"
#include "var/var_period.h"
#include "var/var_string.h"
#include "var/var_uri.h"
#include "var/var_uuid.h"

#endif // _EZ_VARIABLE_H
