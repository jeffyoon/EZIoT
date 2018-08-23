/*
** EZIoT - Main Controller Class
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
#if !defined(_EZ_IOT_H)
#define _EZ_IOT_H
#include "tool/ez_random.h"
#include "tool/ez_timer.h"
#include "tool/ez_uuid.h"

#include "ez_activity.h"
#include "ez_common.h"
#include "ez_console.h"
#include "ez_device.h"
#include "ez_service.h"
#include "ez_variable.h"
#include "iot_root.h"

#define EZ_IOT_MUTEX_TAKE() xSemaphoreTake(_mutexLock, portMAX_DELAY)
#define EZ_IOT_MUTEX_GIVE() xSemaphoreGive(_mutexLock)

namespace EZ
{
    class IOT : protected TIMER
    {
    public:
        typedef enum class SSDP
        {
            NONE,
            ALIVE,
            BYEBYE,
            UPDATE
        } ssdp_method_t;

        typedef enum
        {
            OTAU_STOPPED,
            OTAU_WAITING,
            OTAU_WAITAUTH,
            OTAU_UPDATING
        } otau_state_t;

        static IOT& getInstance(void);
        DEVICE* addDevice(const uint32_t code, DEVICE* newDevice);
        DEVICE& addDevice(const uint32_t code, DEVICE& newDevice);

        void start(uint16_t webPort = EZ_HTTP_PORT);
        void loop(void);
        void stop(void);
        void reset(void);
        void restart(void);

        void wifiCredentials(String ssid, String pass) { wifiCredentials(ssid.c_str(), pass.c_str()); }
        void wifiCredentials(const char* ssid, const char* pass);
        bool wifiConnected(void);
        void wpsSetOFF(void) { _wpsType = EZ_WPS_OFF; }
        void wpsSetPIN(void) { _wpsType = EZ_WPS_PIN; }
        void wpsSetPBC(void) { _wpsType = EZ_WPS_PBC; }

        void timeZone(String tz) { timeZone(tz.c_str()); }
        void timeZone(const char* tz);
        void timeServer(String ts) { timeServer(ts.c_str()); }
        void timeServer(const char* ts);

        void otauCredentials(String pass, uint16_t port = EZ_OTAU_PORT) { otauCredentials(pass.c_str(), port); }
        void otauCredentials(const char* pass, uint16_t port = EZ_OTAU_PORT);
        void otauPort(uint16_t port) { _otauPort.native(port); }

        void mdnsInstance(String name);
        void mdnsService(const char* name, const char* proto, uint16_t port, const char* instName = nullptr,
                         mdns_txt_item_t* txt = nullptr, int len = 0);
        void mdnsServiceTxt(const char* name, const char* proto, const char* key, const char* value);
        void mdnsRemove(const char* name, const char* proto);

        ROOT root;
        CONSOLE& console;

    private:
        VAR::STRING _wifiSSID, _wifiPASS, _timeZone, _timeSvr1, _otauPASS;
        VAR::UI2 _otauPort;

        DEVICE* _headDevice;
        DEVICE* _tailDevice;

        bool _systemStart;
        bool _needRestart;
        bool _wpsConfig;
        bool _smartConfig;

        wps_type_t _wpsType;
        unsigned long _wpsTimeout;
        unsigned long _wifiTimeout;
        unsigned int _ssdpAdvertAge;

        AsyncUDP _ssdpUDP;
        AsyncUDP _otauUDP;
        volatile otau_state_t _otauState;

        IPAddress _otauClientAddr;
        int _otauClientPort;
        int _otauSize;
        int _otauCmd;
        int _otauTimeout;
        String _otauMD5;
        String _otauOnce;
        int _nodeCount;

        SemaphoreHandle_t _mutexLock;
        EventGroupHandle_t _eventGroup;
        const int CONNECTED_BIT = BIT0;
        const int TIMESYNC_BIT = BIT1;
        const int EVENT_BIT = BIT2;
        const int MDNS_BIT = BIT3;
        const int SSDP_BIT = BIT4;
        const int OTAU_BIT = BIT5;
        const int EZIOT_BIT = BIT8;

#if defined(ARDUINO_ARCH_ESP32)
        esp_wps_config_t _cfgWPS;
#endif
        IOT(); // Private, singleton class
        ~IOT();
        IOT(IOT const& copy);            // Not Implemented
        IOT& operator=(IOT const& copy); // Not Implemented

        void _control(DEVICE* dev, iot_control_t mode);
        void _restart(void);
        void _reboot(void);

        int _wifiStart(void);
        int _wifiWait(unsigned long timeout = 0);
        void _wifiHostname(void);
        void _wifiStop(void);
        void _wifiEvents(WiFiEvent_t event, system_event_info_t info);
        void _wpsStart(void);
        String _wpspin2string(uint8_t a[]);

        void _timeSync(void);
        void _sntpStart(void);
        void _sntpStop(void);

        void _mdnsStart(void);
        void _mdnsStop(void);

        void _otauStart(void);
        void _otauStop(void);
        void _otauUpdate(void);
        void _otauProgress(size_t progress, size_t totaul);
        void _otauRequest(AsyncUDPPacket packet);
        int _otauParseInt(AsyncUDPPacket& packet);
        String _otauReadStringUntil(AsyncUDPPacket& packet, char end);

        static void _eventTask(void*);
        void _eventStart(void);
        void _eventStop(void);

        static void _ssdpTask(void*);
        void _ssdpAdvertise(DEVICE* device, ssdp_method_t method);
        void _ssdpStart(void);
        void _ssdpStop(void);
        void _ssdpNotify(DEVICE* dev, ssdp_method_t method, AsyncUDPPacket* packet = nullptr);
        void _ssdpNotify(UPNP::SCP* service, ssdp_method_t method, AsyncUDPPacket* packet = nullptr);
        void _ssdpRespond(const char* loc, const char* usn, const char* stnt, const char* dx, const char* mv,
                          ssdp_method_t method, AsyncUDPPacket* packet = nullptr);
        void _ssdpRequest(AsyncUDPPacket packet);
        void _ssdpRoot(AsyncUDPPacket& packet);
        void _ssdpAll(DEVICE* device, AsyncUDPPacket& packet);
        void _ssdpMatch(DEVICE* device, String& st, AsyncUDPPacket& packet);
        int _ssdpParse(String* token, bool break_on_space, bool break_on_colon, AsyncUDPPacket& packet);
    };

    // Globals
    //
    extern IOT& iot;

} // namespace EZ
#endif // _EZ_IOT_H
