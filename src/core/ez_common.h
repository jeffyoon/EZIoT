/*
** EZIoT - Common Includes/Equates/Defintions and Functions
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
#ifndef _EZ_COMMON_H
#define _EZ_COMMON_H
#include <Arduino.h>
#include <WString.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>

#include <esp_log.h>
#include <esp_system.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>

#include <FS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <mdns.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <AsyncUDP.h>
#include <esp_wps.h>
#endif

#include <Print.h>

// These should be defined at build time
#if !defined(ARDUINO_BOARD)
#define ARDUINO_BOARD "Espressif ESP32"
#endif

#if !defined(ARDUINO_VARIANT)
#define ARDUINO_VARIANT "esp32"
#endif

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#undef max
#define max(a, b) ((a > b) ? a : b)

#ifndef nvs_error
#define nvs_error(e) (((e) > ESP_ERR_NVS_BASE) ? nvs_errors[(e) & ~(ESP_ERR_NVS_BASE)] : nvs_errors[0])
#endif

#define EZ_BOARD ARDUINO_BOARD
#define EZ_VARIANT ARDUINO_VARIANT
#define EZ_VERSION "0.0.0.1"

#define EZ_DEFAULT_TIMEZONE "UTC"
#define EZ_DEFAULT_TIMESERVER "pool.ntp.org"

#define EZ_HTTP_PORT 80
#define EZ_OTAU_PORT 3232

#define EZ_WPS_OFF WPS_TYPE_DISABLE
#define EZ_WPS_PIN WPS_TYPE_PIN
#define EZ_WPS_PBC WPS_TYPE_PBC
#define EZ_WPS_MODE EZI_WPS_PBC

#define EZ_MAX_SSID 31
#define EZ_MAX_PASS 63
#define EZ_MAX_HOST 62
#define EZ_MAX_NAME 40

#define EZ_SSDP_METHOD_SIZE 10
#define EZ_SSDP_URI_SIZE 2
#define EZ_SSDP_BUFFER_SIZE 64
#define EZ_SSDP_ADVERT_AGE 1800
#define EZ_SSDP_MULTICAST_TTL 2
#define EZ_SSDP_MULTICAST_PORT 1900
#define EZ_SSDP_MULTICAST_ADDR IPAddress(239, 255, 255, 250)

#define EZ_UPNP_UUID_DEVICE_PREFIX "50fbbdab-5418-41c1-a96d-"
#define EZ_UPNP_SCHEMA_DEVICE_XMLNS "urn:schemas-upnp-org:device-1-0"
#define EZ_UPNP_SCHEMA_DEVICE_BASIC1 "urn:schemas-upnp-org:device:Basic:1.0"
#define EZ_UPNP_SCHEMA_SERVICE_XMLNS "urn:schemas-upnp-org:service-1-0"

#define EZ_UPNP_MODEL_URL "http://www.espressif.com/"
#define EZ_UPNP_MANUFACTURER_URL "http://github.com/EZIoT/EZIoT"

#define EZ_UPNP_NULL_STATUS_PREFIX "A_ARG_TYPE_"
#define EZ_UPNP_MAX_ARGS 6
#define EZ_UPNP_MAX_SUBSCRIPTIONS 5
#define EZ_UPNP_SUBSCRIPTION_TIMEOUT 1800 // Minimum allowed as per UPnP Spec.

#define EZ_SOAP_ERROR_NONE 0
#define EZ_SOAP_ERROR_INVALID_ACTION 401
#define EZ_SOAP_ERROR_INVALID_ARGS 402
#define EZ_SOAP_ERROR_ACTION_FAILED 501
#define EZ_SOAP_ERROR_INVALID_VALUE 600
#define EZ_SOAP_ERROR_OUT_OF_RANGE 601
#define EZ_SOAP_ERROR_NOT_IMPLEMENTED 602
#define EZ_SOAP_ERROR_OUT_OF_MEMORY 603
#define EZ_SOAP_ERROR_INTERVENTION 604
#define EZ_SOAP_ERROR_STRING_LENGTH 605

namespace EZ
{
    extern const char* iotTag;
    extern const char* nvs_errors[];

    extern String dateRFC3339(void);
    extern String dateRFC3339(time_t* t);
    extern String dateRFC3339(struct tm* tm);
    extern String dateRFC1123(void);
    extern String dateRFC1123(time_t* t);
    extern String dateRFC1123(struct tm* tm);

    // extern String xmlTag(String tag, String value, bool emptyTag = false);
    extern String xmlTag(const char* tag, String value, bool emptyTag = false);
    extern String parseURN(String urn);
    extern uint32_t foldString(String s, int M);

} // namespace EZ

#endif // _EZ_COMMON_H
