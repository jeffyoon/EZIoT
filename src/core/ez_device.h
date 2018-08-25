/*
** EZIoT - Device Base Class
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
#if !defined(_EZ_DEVICE_H)
#define _EZ_DEVICE_H
#include "ez_common.h"
#include "ez_config.h"
#include "ez_http.h"
#include "ez_service.h"
#include "upnp_scp.h"

namespace EZ
{
    // TODO: Moved into the DEVICE class!
    enum class CONTROL
    {
        INIT,
        START,
        LOOP,
        STOP
    };

    typedef CONTROL iot_control_t;

    class DEVICE : protected HTTP::HANDLER
    {
        friend class IOT;

    public:
        virtual ~DEVICE();
        DEVICE() : DEVICE(0) {}
        DEVICE(uint16_t port);

        void ssdpAdverts(bool advertise) { _ssdpAdverts = advertise; }
        bool ssdpAlive(void) { return _ssdpAdverts; }
        virtual bool ssdpMatch(String& st);
        virtual String ssdpModel(void);
        virtual String ssdpExtra(void) { return _ssdpExtra; }

        virtual String upnpXML(bool root = false);
        virtual String upnpXMLNS(void);
        virtual String upnpVersionMajor(void) { return String(_upnpVersionMajor); }
        virtual String upnpVersionMinor(void) { return String(_upnpVersionMinor); }
        virtual String upnpDeviceType(void);
        virtual String upnpFriendlyName(void);
        virtual void upnpFriendlyName(const char* name);
        virtual void upnpFriendlyName(String name) { upnpFriendlyName(name.c_str()); }
        virtual String upnpManufacturer(void);
        virtual String upnpManufacturerURL(void);
        virtual String upnpModelDescription(void);
        virtual String upnpModelName(void);
        virtual String upnpModelNumber(void);
        virtual String upnpModelURL(void);
        virtual String upnpSerialNumber(void);
        virtual String upnpUDN(void);
        virtual String upnpUPC(void);
        virtual String upnpUUID(void);
        virtual String upnpExtra(void) { return _upnpExtra; }
        virtual String upnpServer(void);
        virtual String urlSchema(bool pathOnly = true);
        virtual String urlPresentation(bool pathOnly = true);

        String urlBase(String path) { return urlBase(path.c_str()); }
        String urlBase(const char* path = nullptr);

        uint16_t httpPort(void);

        DEVICE* addDevice(const uint32_t code, DEVICE* newDevice);
        DEVICE& addDevice(const uint32_t code, DEVICE& newDevice);
        DEVICE* homeDevice(void) const { return _homeDevice; }
        DEVICE* headDevice(void) const { return _headDevice; }
        DEVICE* tailDevice(void) const { return _tailDevice; }
        DEVICE* prevDevice(void) const { return _prevDevice; }
        DEVICE* nextDevice(void) const { return _nextDevice; }
        SERVICE* addService(SERVICE* newService);
        SERVICE& addService(SERVICE& newService);
        SERVICE* headService(void) const { return _headService; }
        SERVICE* tailService(void) const { return _tailService; }

    protected:
        uint16_t _upnpVersionMajor;
        uint16_t _upnpVersionMinor;
        const char* _upnpXMLNS = nullptr;
        const char* _upnpDeviceType = nullptr;
        const char* _upnpManufacturer = nullptr;
        const char* _upnpManufacturerURL = nullptr;
        const char* _upnpModelDescription = nullptr;
        const char* _upnpModelName = nullptr;
        const char* _upnpModelNumber = nullptr;
        const char* _upnpModelURL = nullptr;
        const char* _upnpSerialNumber = nullptr;
        const char* _upnpUDN = nullptr;
        const char* _upnpUPC = nullptr;
        const char* _upnpExtra = nullptr;
        const char* _ssdpExtra = nullptr;

        DEVICE* _homeDevice;
        DEVICE* _prevDevice;
        DEVICE* _nextDevice;
        DEVICE* _headDevice;
        DEVICE* _tailDevice;
        SERVICE* _headService;
        SERVICE* _tailService;

        HTTP::SERVER* _httpServer;
        uint16_t _httpPort;

        uint32_t _iotCode;

        CONFIG _config;
        VAR::STRING _upnpFriendlyName;
        VAR::UUID _upnpUUID;

        virtual void _control(iot_control_t mode);
        void _httpSetup(HTTP::SERVER* server);
        virtual void _http404(HTTP::SERVER& server);
        virtual bool _httpAccept(HTTP::METHOD method, String uri);
        virtual bool _httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri);

    private:
        bool _ssdpAdverts;

        DEVICE(DEVICE const& copy);            // Not Implemented
        DEVICE& operator=(DEVICE const& copy); // Not Implemented
    };
} // namespace EZ
#endif // _EZ_DEVICE_H
/******************************************************************************/
