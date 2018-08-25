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
#include "ez_device.h"
#include "iot.h"

using namespace EZ;

static const char _device_icon_list[] =
    "<iconList>\r\n"
    "<icon>"
    "<mimetype>image/png</mimetype>"
    "<width>128</width>"
    "<height>128</height>"
    "<depth>32</depth>\r\n"
    "<url>http://raw.githubusercontent.com/EZIoT/EZIoT/master/img/iot128.png</url>\r\n"
    "</icon>\r\n"
    "<icon>"
    "<mimetype>image/png</mimetype>"
    "<width>48</width>"
    "<height>48</height>"
    "<depth>32</depth>\r\n"
    "<url>http://raw.githubusercontent.com/EZIoT/EZIoT/master/img/iot48.png</url>\r\n"
    "</icon>\r\n"
    "</iconList>\r\n";

DEVICE::DEVICE(uint16_t port)
    : _upnpVersionMajor(1), _upnpVersionMinor(0), _upnpXMLNS(nullptr), _upnpDeviceType(nullptr),
      _upnpManufacturer(nullptr), _upnpManufacturerURL(nullptr), _upnpModelDescription(nullptr),
      _upnpModelName(nullptr), _upnpModelNumber(nullptr), _upnpModelURL(nullptr), _upnpSerialNumber(nullptr),
      _upnpUDN(nullptr), _upnpUPC(nullptr), _upnpExtra(nullptr), _ssdpExtra(nullptr), _homeDevice(nullptr),
      _prevDevice(nullptr), _nextDevice(nullptr), _headDevice(nullptr), _tailDevice(nullptr), _headService(nullptr),
      _tailService(nullptr), _httpServer(nullptr), _httpPort(port),
      _upnpFriendlyName("FriendlyName", false, true, "", 32), _upnpUUID("uuid:", false, true, false), _ssdpAdverts(true)
{
    _config.addActivity(_upnpFriendlyName);
    _config.addActivity(_upnpUUID);
    addService(_config);
}

DEVICE::~DEVICE() {}

/*
** SSDP Model
*/
String DEVICE::ssdpModel(void)
{
    String model("freeRTOS/");
    model.concat(ESP.getSdkVersion());

    return model;
}

/*
** SSDP Matcher
*/
bool DEVICE::ssdpMatch(String& st)
{
    String _dt(upnpDeviceType());
    String _st(st);
    int w;

    _dt.toLowerCase();
    _st.toLowerCase();

    // Fudge for Wemo's
    if ((w = _st.lastIndexOf("**")) > 0)
        _st = _st.substring(0, w);

    if (_dt.startsWith(_st))
    {
        // ESP_LOGD(eziotString, "%s == %s", st.c_str(), upnpDeviceType().c_str());
        return true;
    }

    return false;
}

/*
** UPNP
*/
String DEVICE::upnpXML(bool root)
{
    String xml("");

    if (root)
    {
        xml.concat("<?xml version=\"1.0\"?>\r\n");
        xml.concat("<root xmlns=\"" + upnpXMLNS() + "\">\r\n");
        xml.concat("<specVersion>\r\n");
        xml.concat(xmlTag("major", upnpVersionMajor()));
        xml.concat(xmlTag("minor", upnpVersionMinor()));
        xml.concat("</specVersion>\r\n");

        if (_upnpVersionMajor < 2 && _upnpVersionMinor < 1)
            xml.concat(xmlTag("URLBase", urlBase(), false));
    }

    xml.concat("<device>\r\n");
    xml.concat(xmlTag("deviceType", upnpDeviceType(), true));
    xml.concat(xmlTag("friendlyName", upnpFriendlyName(), true));
    xml.concat(xmlTag("manufacturer", upnpManufacturer(), true));
    xml.concat(xmlTag("manufacturerURL", upnpManufacturerURL()));
    xml.concat(xmlTag("modelDescription", upnpModelDescription()));
    xml.concat(xmlTag("modelName", upnpModelName(), true));
    xml.concat(xmlTag("modelNumber", upnpModelNumber()));
    xml.concat(xmlTag("modelURL", upnpModelURL()));
    xml.concat(xmlTag("serialNumber", upnpSerialNumber()));
    xml.concat(xmlTag("UDN", upnpUDN(), true));
    xml.concat(xmlTag("UPC", upnpUPC()));
    xml.concat(upnpExtra());

    // xml.concat(_device_icon_list);

    // serviceList
    //
    if (_headService)
    {
        SERVICE* service = _headService;
        bool closeTag = false;

        do
        {
            if (service->mode() == SERVICE::MODE::UPNP)
            {
                UPNP::SCP* upnp = reinterpret_cast<UPNP::SCP*>(service);

                if (!closeTag)
                {
                    xml.concat("<serviceList>\r\n");
                    closeTag = true;
                }
                xml.concat("<service>\r\n");
                xml.concat(xmlTag("serviceType", upnp->upnpServiceType(), true));
                xml.concat(xmlTag("serviceId", upnp->upnpServiceId(), true));
                xml.concat(xmlTag("controlURL", upnp->urlControl(), true));
                xml.concat(xmlTag("eventSubURL", upnp->urlEvents(), true));
                xml.concat(xmlTag("SCPDURL", upnp->urlSchema(), true));
                xml.concat("</service>\r\n");
            }
        } while ((service = service->nextService()));

        if (closeTag)
            xml.concat("</serviceList>\r\n");
    }

    // deviceList
    //
    if (_headDevice)
    {
        DEVICE* device = _headDevice;
        bool closeTag = false;

        do
        {
            String devXML = device->upnpXML(false);

            if (devXML != "" && !closeTag)
            {
                xml.concat("<deviceList>\r\n");
                closeTag = true;
            }
            xml.concat(devXML);
        } while ((device = device->nextDevice()));

        if (closeTag)
            xml.concat("</deviceList>\r\n");
    }

    xml.concat(xmlTag("presentationURL", urlPresentation()));
    xml.concat("</device>\r\n");

    if (root)
        xml.concat("</root>\r\n");

    return xml;
}

String DEVICE::upnpXMLNS(void)
{
    if (_upnpXMLNS)
        return _upnpXMLNS;
    return EZ_UPNP_SCHEMA_DEVICE_XMLNS;
}

String DEVICE::upnpDeviceType(void)
{
    if ((_upnpDeviceType) && strncasecmp("urn:", _upnpDeviceType, 4) != 0)
    {
        String type("urn:EZIoT:device:");
        type.concat(_upnpDeviceType);
        return type;
    }

    return _upnpDeviceType;
}

String DEVICE::upnpFriendlyName(void) { return _upnpFriendlyName.value(); }

void DEVICE::upnpFriendlyName(const char* name) { _upnpFriendlyName.value(name); }

String DEVICE::upnpManufacturer(void)
{
    if (_upnpManufacturer)
        return _upnpManufacturer;
    return iotTag;
}

String DEVICE::upnpManufacturerURL(void)
{
    if (_upnpManufacturerURL)
        return _upnpManufacturerURL;
    return EZ_UPNP_MANUFACTURER_URL;
}

String DEVICE::upnpModelDescription(void)
{
    if (_upnpModelDescription)
        return _upnpModelDescription;
    return EZ_BOARD;
}

String DEVICE::upnpModelName(void)
{
    if (_upnpModelName)
        return _upnpModelName;
    return EZ_VARIANT;
}

String DEVICE::upnpModelNumber(void)
{
    if (_upnpModelNumber)
        return _upnpModelNumber;

    uint8_t rev;

    switch ((rev = ESP.getChipRevision()))
    {
        case 0:
            return "ESP32D0WDQ6";
        case 1:
            return "ESP32D0WDQ5";
        case 2:
            return "ESP32D2WDQ5";
    }

    return "ESP32";
}

String DEVICE::upnpModelURL(void)
{
    if (_upnpModelURL)
        return _upnpModelURL;
    return EZ_UPNP_MODEL_URL;
}

String DEVICE::upnpSerialNumber(void)
{
    if (_upnpSerialNumber)
        return _upnpSerialNumber;

    char serial[20];

    sprintf(serial, "%" PRIu64, ESP.getEfuseMac());
    return serial;
}

String DEVICE::upnpUDN(void)
{
    if (!_upnpUDN)
    {
        String udn("uuid:");
        udn.concat(upnpUUID());
        return udn;
    }

    return _upnpUDN;
}

String DEVICE::upnpUPC(void) { return _upnpUPC; }

String DEVICE::upnpUUID(void)
{
    if (_upnpUUID.isZero())
        return _upnpUUID.makeV4();
    return _upnpUUID.value();
}

String DEVICE::upnpServer(void)
{
    char server[100];
    uint16_t vMaj = _upnpVersionMajor;
    uint16_t vMin = _upnpVersionMinor;

    if (vMaj > 1)
        vMaj = vMin = 1;

    snprintf(server, sizeof(server), "EZIoT/%s, UPnP/%u.%u, %s", EZ_VERSION, vMaj, vMin, ssdpModel().c_str());

    return server;
}

String DEVICE::urlSchema(bool pathOnly)
{
    String url = pathOnly ? "" : urlBase();

    url.concat("/upnp/" + upnpUUID() + "/device.xml");
    url.replace("-", "");
    url.toLowerCase();

    return url;
}

String DEVICE::urlPresentation(bool pathOnly)
{
    String url = pathOnly ? "" : urlBase();

    url.concat("/eziot/index.html");

    return url;
}

String DEVICE::urlBase(const char* path)
{
    String url("http://");
    uint16_t port = httpPort();

    url.concat(WiFi.localIP().toString());

    if (port != 0)
    {
        url.concat(":");
        url.concat(port);
    }

    if ((path) && *path != '\0')
        url.concat(path);

    return url;
}

/*
** Add (upnp) embedded device
*/
DEVICE& DEVICE::addDevice(const uint32_t code, DEVICE& newDevice)
{
    (void)addDevice(code, &newDevice);
    return newDevice;
}

DEVICE* DEVICE::addDevice(const uint32_t code, DEVICE* newDevice)
{
    ESP_LOGI(iotTag, "Add Embedded Device: %s", newDevice->upnpDeviceType().c_str());

    if ((newDevice) && !newDevice->_homeDevice && !newDevice->_iotCode)
    {
        if (_headDevice == nullptr)
        {
            newDevice->_nextDevice = nullptr;
            newDevice->_prevDevice = nullptr;
            _headDevice = newDevice;
            _tailDevice = newDevice;
        }
        else
        {
            newDevice->_nextDevice = nullptr;
            newDevice->_prevDevice = _tailDevice;
            newDevice->_prevDevice->_nextDevice = newDevice;
            _tailDevice = newDevice;
        }

        ESP_LOGI(iotTag, "Added Embedded Device: %s", newDevice->upnpDeviceType().c_str());

        newDevice->_homeDevice = this;
        newDevice->_iotCode = code;
    }

    return newDevice;
}

/*
** Add service
*/
SERVICE* DEVICE::addService(SERVICE* newService)
{
    if ((newService) && !newService->_baseDevice)
    {
        if (_headService == nullptr)
        {
            newService->_nextService = nullptr;
            newService->_prevService = nullptr;
            _headService = newService;
            _tailService = newService;
        }
        else
        {
            newService->_nextService = nullptr;
            newService->_prevService = _tailService;
            newService->_prevService->_nextService = newService;
            _tailService = newService;
        }

        newService->_baseDevice = this;
    }

    return newService;
}

SERVICE& DEVICE::addService(SERVICE& newService)
{
    (void)addService(&newService);
    return newService;
}

uint16_t DEVICE::httpPort(void)
{
    if (_httpServer)
        return _httpServer->httpPort();
    return _httpPort;
}

/*
** Controller
*/
void DEVICE::_control(iot_control_t mode)
{
    /*
    switch (mode)
    {
        case CONTROL::INIT:
            break;
        case CONTROL::START:
            break;
        case CONTROL::STOP:
            break;
        case CONTROL::LOOP:
            break;
    }
    */
}

/*
** HTTP Tools
*/
void DEVICE::_httpSetup(HTTP::SERVER* server)
{
    static const char* headerkeys[] = {"HOST",     "USER-AGENT", "SOAPAction", "SID",
                                       "CALLBACK", "NT",         "TIMEOUT",    "STATEVAR"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    if (!server)
        return;

    server->collectHeaders(headerkeys, headerkeyssize);
    server->on404(std::bind(&DEVICE::_http404, this, std::placeholders::_1));
}

bool DEVICE::_httpAccept(HTTP::METHOD method, String uri)
{
    if (method == HTTP::METHOD::GET && uri == urlSchema(true))
        return true;

    return false;
}

bool DEVICE::_httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri)
{
    if (method == HTTP::METHOD::GET && uri == urlSchema(true))
    {
        server.sendHeader("Server", upnpServer().c_str());
        server.sendHeader("Date", dateRFC1123());
        server.sendHeader("Content-Language", "en");

        return server.send(200, MIME_TYPE_XML, upnpXML(true));
    }

    return server.send(501); // Not Implemented
}

/*
** HTTP Handlers
*/
void DEVICE::_http404(HTTP::SERVER& server)
{
    String message = iotTag;
    message += ": File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += server.methodToString(server.method());
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(404, MIME_TYPE_TEXT, message);
}
