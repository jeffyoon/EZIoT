/*
** EZIoT - IOT Controller: WiFi Stack
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
#include "ez_service.h"
#include "iot.h"

using namespace EZ;

/*
** WiFi Tools
*/
void IOT::wifiCredentials(const char* ssid, const char* pass)
{
    _wifiSSID.value(ssid);
    _wifiPASS.value(pass);

    /*
        if (_systemStart)
        {
            _wifiSSID.value(ssid);
            _wifiPASS.value(pass);
        }
        else
        {
            _wifiSSID._defaultValue = ssid;
            _wifiPASS._defaultValue = pass;
        }
    */
}

bool IOT::wifiConnected(void)
{
    EventBits_t bits = xEventGroupGetBits(_eventGroup);

    return (bits & CONNECTED_BIT);
}

/*
** WiFi Control
*/
int IOT::_wifiStart(void)
{
    String ssid = _wifiSSID.value();
    String pass = _wifiPASS.value();
    int result;

    // Set (any) static IP settings
    // if (_wifiStaticIP)
    {
        // ESP_LOGI(eziotString, "Set static WiFi configuration");
        // WiFi.config(_wifiStaticIP->ip, _wifiStaticIP->gw, _wifiStaticIP->sn, _wifiStaticIP->ns1, _wifiStaticIP->ns2);
    }

    // Avoid race conditions
    if (WiFi.status() == WL_CONNECTED)
    {
        console.printf(LOG::WARNING, "WiFi already connected, bailing out");
        return WL_CONNECTED;
    }

    WiFi.onEvent(std::bind(&IOT::_wifiEvents, this, std::placeholders::_1, std::placeholders::_2));
    WiFi.mode(WIFI_MODE_STA);

    // check if we have ssid and pass and force those, if not, try with last saved values
    if (ssid != "")
    {
        console.printf(LOG::INFO1, "Connect WiFi (%s)", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());
    }
    else if (WiFi.status() != WL_NO_SHIELD)
    {
        if (WiFi.SSID() != "")
        {
            console.printf(LOG::INFO1, "Re-connecting WiFi (%s)", WiFi.SSID().c_str());
#if defined(ESP8266)
            // trying to fix connection in progress hanging
            ETS_UART_INTR_DISABLE();
            wifi_station_disconnect();
            ETS_UART_INTR_ENABLE();
#else
            // esp_wifi_disconnect();
            WiFi.disconnect();
#endif

            WiFi.begin();
        }
        else
        {
            if (!_smartConfig)
            {
                console.printf(LOG::INFO1, "WiFi SmartConfig Enabled.");
                WiFi.beginSmartConfig();
                _smartConfig = true;
            }

            if (!_wpsConfig && _wpsType != WPS_TYPE_DISABLE && pass == "")
                _wpsStart();
        }
    }

    if ((result = _wifiWait(_wifiTimeout)) != WL_CONNECTED)
    {
        console.printf(LOG::ERROR, "WiFi not connected! (%d)", result);

        if (!_wpsConfig && _wpsType != WPS_TYPE_DISABLE && pass == "")
        {
            _wpsStart();
            result = _wifiWait(_wpsTimeout);
        }
    }

    if (result == WL_CONNECTED)
    {
        if (_smartConfig)
        {
            if (WiFi.smartConfigDone())
            {
                console.printf(LOG::INFO1, "WiFi Smart Config done");
                ESP_LOGV(eziotString, "WiFi SSID : %s", WiFi.SSID().c_str());
                ESP_LOGV(eziotString, "WiFi PSK  : %s", WiFi.psk().c_str());
                // Save SSID and PSK to NVS?

                if (_wpsConfig)
                {
                    esp_wifi_wps_disable();
                    _wpsConfig = false;
                }
            }
            _smartConfig = false;
            WiFi.stopSmartConfig();
        }

        if (_wpsConfig)
        {
            console.printf(LOG::INFO1, "WiFi WPS Config done");
            ESP_LOGV(eziotString, "WiFi SSID : %s", WiFi.SSID().c_str());
            ESP_LOGV(eziotString, "WiFi PSK  : %s", WiFi.psk().c_str());

            _wpsConfig = false;
            if (_smartConfig)
            {
                WiFi.stopSmartConfig();
                _smartConfig = false;
            }
        }
    }

    return result;
}

void IOT::_wifiHostname(void)
{
    char hn[EZ_MAX_HOST + 1] = {0};
    uint8_t mac[6];

    WiFi.macAddress(mac);
    snprintf(hn, EZ_MAX_HOST, "%s-%02x%02x%02x%02x%02x%02x", iotTag, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    String hostName(hn);

    hostName.toLowerCase();
    WiFi.setHostname(hostName.c_str());
}

int IOT::_wifiWait(unsigned long timeout)
{
    console.printf(LOG::INFO1, "Waiting for WiFi connection...");

    if (timeout == 0)
        return WiFi.waitForConnectResult();

    unsigned long start = millis();
    boolean keepConnecting = true;
    int status;

    do
    {
        status = WiFi.status();

        if (millis() > start + timeout)
        {
            keepConnecting = false;
            console.printf(LOG::WARNING, "WiFi connection timed out");
        }

        if (status == WL_CONNECTED /*|| status == WL_CONNECT_FAILED*/)
        {
            keepConnecting = false;
        }
        else
            delay(200);
        yield();
    } while (keepConnecting);

    return status;
}

void IOT::_wifiStop(void)
{
    _mdnsStop();
    _ssdpStop();
    _sntpStop();
    _otauStop();

#if defined(ARDUINO_ARCH_ESP32)
    WiFi.disconnect(true);
#else
    // trying to fix connection in progress hanging
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
#endif
    _smartConfig = false;
    _wpsConfig = false;
    xEventGroupClearBits(_eventGroup, CONNECTED_BIT | TIMESYNC_BIT);
}

void IOT::_wifiEvents(WiFiEvent_t event, system_event_info_t info)
{
#if defined(ARDUINO_ARCH_ESP32)
    switch (event)
    {
            // SYSTEM_EVENT_AP_START,           /**< ESP32 soft-AP start */
            // SYSTEM_EVENT_AP_STOP,            /**< ESP32 soft-AP stop */
            // SYSTEM_EVENT_AP_STACONNECTED,    /**< a station connected to ESP32 soft-AP */
            // SYSTEM_EVENT_AP_STADISCONNECTED, /**< a station disconnected from ESP32 soft-AP */
            // SYSTEM_EVENT_AP_STAIPASSIGNED,   /**< ESP32 soft-AP assign an IP to a connected station */
            // SYSTEM_EVENT_AP_PROBEREQRECVED,  /**< Receive probe request packet in soft-AP interface */
            // SYSTEM_EVENT_GOT_IP6,          /**< ESP32 station or ap or ethernet interface v6IP addr is preferred */
            // SYSTEM_EVENT_ETH_START,        /**< ESP32 ethernet start */
            // SYSTEM_EVENT_ETH_STOP,         /**< ESP32 ethernet stop */
            // SYSTEM_EVENT_ETH_CONNECTED,    /**< ESP32 ethernet phy link up */
            // SYSTEM_EVENT_ETH_DISCONNECTED, /**< ESP32 ethernet phy link down */
            // SYSTEM_EVENT_ETH_GOT_IP,       /**< ESP32 ethernet got IP from connected AP */

        case SYSTEM_EVENT_WIFI_READY:
            console.printf(LOG::INFO1, "WiFi Ready");
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            console.printf(LOG::INFO1, "WiFi Scan Complete");
            break;
        case SYSTEM_EVENT_STA_START:
            console.printf(LOG::INFO1, "Station Mode Started");
            _wifiHostname();
            break;
        case SYSTEM_EVENT_STA_STOP:
            console.printf(LOG::WARNING, "Station Mode Stopped");
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            console.printf(LOG::INFO1, "WiFi SSID   : %s", WiFi.SSID().c_str());
            console.printf(LOG::INFO1, "WiFi RSSI   : %d", WiFi.RSSI());
            console.printf(LOG::INFO1, "Hostname    : %s", WiFi.getHostname());
            console.printf(LOG::INFO1, "MAC Address : %s", WiFi.macAddress().c_str());
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            console.printf(LOG::WARNING, "Disconnected from station, attempting reconnection");
            xEventGroupClearBits(_eventGroup, CONNECTED_BIT | TIMESYNC_BIT);
            _otauStop();
            _mdnsStop();
            _ssdpStop();
            _sntpStop();
            WiFi.reconnect();
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            console.printf(LOG::WARNING, "Station Auth Mode Change");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            if (info.got_ip.ip_changed)
                ESP_LOGW(eziotString, "IP Address has changed");
            console.printf(LOG::INFO1, "IP Address  : %s", WiFi.localIP().toString().c_str());
            console.printf(LOG::INFO1, "Subnet Mask : %s", WiFi.subnetMask().toString().c_str());
            console.printf(LOG::INFO1, "IP Gateway  : %s", WiFi.gatewayIP().toString().c_str());
            console.printf(LOG::INFO1, "DNS Server  : %s", WiFi.dnsIP().toString().c_str());
            xEventGroupSetBits(_eventGroup, CONNECTED_BIT);
            _sntpStart();
            _mdnsStart();
            _otauStart();
            _ssdpStart();
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            ESP_LOGW(eziotString, "Lost IP Address!");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            console.printf(LOG::INFO1, "WPS Successfull, connecting to: %s", WiFi.SSID().c_str());
            ESP_LOGI(eziotString, "WiFi PSK  : %s", WiFi.psk().c_str());
            esp_wifi_wps_disable();
            delay(10);
            WiFi.begin();
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            console.printf(LOG::WARNING, "WPS Failed, retrying");
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&_cfgWPS);
            esp_wifi_wps_start(0);
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            console.printf(LOG::WARNING, "WPS Timedout, retrying");
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&_cfgWPS);
            esp_wifi_wps_start(0);
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            console.printf(LOG::INFO1, "WPS PIN: %s", _wpspin2string(info.sta_er_pin.pin_code).c_str());
            break;
        default:
            ESP_LOGI(eziotString, "Unknown Event: %d", event);
            break;
    }
#endif
}

void IOT::_wpsStart(void)
{
    if (_wpsType == WPS_TYPE_DISABLE || _wpsConfig)
        return;

#if defined(ARDUINO_ARCH_ESP32)
    console.printf(LOG::INFO1, "WiFi WPS Config Enabled.");
    _cfgWPS.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
    _cfgWPS.wps_type = _wpsType;
    strcpy(_cfgWPS.factory_info.manufacturer, iotTag);
    strcpy(_cfgWPS.factory_info.model_number, EZ_VARIANT);
    strcpy(_cfgWPS.factory_info.model_name, EZ_BOARD);
    strcpy(_cfgWPS.factory_info.device_name, WiFi.getHostname());

    esp_wifi_wps_enable(&_cfgWPS);
    esp_wifi_wps_start(0); // Timeout=0 for now
    _wpsConfig = true;
#elif defined(ARDUINO_ARCH_ESP8266)
    console.printf(LOG::INFO1, "WiFi WPS Config Enabled.");
    WiFi.beginWPSConfig();
#endif
}

String IOT::_wpspin2string(uint8_t a[])
{
    char wps_pin[9];

    for (int i = 0; i < 8; i++)
    {
        wps_pin[i] = a[i];
    }

    wps_pin[8] = '\0';
    return (String)wps_pin;
}
