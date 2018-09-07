/*
** EZIoT - IOT Controller
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
#include "ez_http.h"
#include "iot.h"

using namespace EZ;

static CONSOLE _defCon(Serial);

const char* EZ::iotTag = "EZIoT";
IOT& EZ::iot = IOT::getInstance();

/*
** Global Instantiated IOT Class
*/
IOT& IOT::getInstance()
{
    static IOT instance;
    return instance;
}

/*
** IOT Class
*/
IOT::IOT()
    : root(ROOT()), console(_defCon), _wifiSSID("wifiSSID", false, true, "", EZ_MAX_SSID),
      _wifiPASS("wifiPASS", false, true, "", EZ_MAX_PASS), _timeZone("tz", false, true, EZ_DEFAULT_TIMEZONE, 32),
      _timeSvr1("tzs1", false, true, EZ_DEFAULT_TIMESERVER, EZ_MAX_HOST),
      _otauPASS("otauPASS", false, true, "", EZ_MAX_PASS), _otauPort("otauPort", false, true, EZ_OTAU_PORT),
      _headDevice(nullptr), _tailDevice(nullptr), _systemStart(false), _needRestart(false)
{
    _mutexLock = xSemaphoreCreateMutex();
    _eventGroup = xEventGroupCreate();
    _headDevice = _tailDevice = &root;
}

IOT::~IOT() {}

/*
** Device Tools
*/
DEVICE& IOT::addDevice(const uint32_t code, DEVICE& newDevice)
{
    (void)addDevice(code, &newDevice);
    return newDevice;
}

DEVICE* IOT::addDevice(const uint32_t code, DEVICE* newDevice)
{
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

        // newDevice->_homeDevice = &root;
        newDevice->_iotCode = code;
    }

    return newDevice;
}

/*
** Start the IOT device
*/
void IOT::start(uint16_t webPort)
{
    uint64_t _chipID = ESP.getEfuseMac();

    if (_systemStart)
        return;
    _systemStart = true;

    console.printf(LOG::TITLE, "%s, v%s", iotTag, EZ_VERSION);
    console.printf(LOG::INFO1, "Chip (%d): %04X%08X @ %d MHz - Flash: %d @ %d, SDK: %s", ESP.getChipRevision(),
                   (uint16_t)(_chipID >> 32), (uint32_t)_chipID, ESP.getCpuFreqMHz(), ESP.getFlashChipSize(),
                   ESP.getFlashChipSpeed(),
                   ESP.getSdkVersion()); // const char* esp_get_idf_version

    console.printf(LOG::INFO1, "Free Heap %d", ESP.getFreeHeap());
    console.printf(LOG::INFO1, "Booting ...");

    // Start the flash driver
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    _eventStart();

    root._httpPort = webPort;
    _nodeCount = 0;
    _control(_headDevice, CONTROL::INIT);

    ESP_LOGV(iotTag, "Node Count: %d", _nodeCount);

    if (_wifiStart() != WL_CONNECTED)
    {
        // Start captive config portal??
        _needRestart = true;
    }
    else
    {
        EventBits_t bits = xEventGroupWaitBits(_eventGroup, CONNECTED_BIT, false, true, portMAX_DELAY);

        if (bits & CONNECTED_BIT)
        {
            console.printf(LOG::TITLE, "** System Services Ready! **");
            xEventGroupSetBits(_eventGroup, EZIOT_BIT);
            timerPeriod(60000);
            console.printf(LOG::TITLE, "** Starting Device(s) **");
            _control(_headDevice, CONTROL::START);
            console.printf(LOG::INFO1, "Free Heap %d", ESP.getFreeHeap());
        }
        else
            _needRestart = true;
    }
}

/*
** IOT device loop
*/
void IOT::loop(void)
{
    EventBits_t bits = xEventGroupGetBits(_eventGroup);

    if (_needRestart)
    {
        _restart();
        return; // Shouldn't get here!
    }

    if (bits & CONNECTED_BIT)
    {
        // Our watchdog timer
        //
        if (timerExpired())
        {
            console.printf(LOG::TITLE, "** Heartbeat **");
            timerReset();

            // Pulse watchdog pin!

            // May not need this, but keep just in case!
            time_t _timeTick;
            struct tm _timeInfo;

            time(&_timeTick);
            localtime_r(&_timeTick, &_timeInfo);

            if (_timeInfo.tm_year < (2016 - 1900))
                _timeSync();
        }
    }

    if (!(bits & OTAU_BIT) && bits & EZIOT_BIT)
    {
        EZ_IOT_MUTEX_TAKE();
        _control(_headDevice, CONTROL::LOOP);
        EZ_IOT_MUTEX_GIVE();
    }
}

/*
** Stop the IOT device
*/
void IOT::stop(void)
{
    if (!_systemStart)
        return;

    _systemStart = false;
    console.printf(LOG::INFO1, "** System Shutdown **");
    xEventGroupClearBits(_eventGroup, EZIOT_BIT);
    _control(_headDevice, CONTROL::STOP);
    _eventStop();
    _wifiStop();
    console.printf(LOG::TITLE, "** System Stopped! **");
}

/*
** Reset (clear) NVS and restart device
*/
void IOT::reset(void)
{
    console.printf(LOG::TITLE, "** System Reset **");
    // TODO: Clear NVS
    stop();
    _reboot();
}

/*
** Restart the IOT device
*/
void IOT::restart(void) { _needRestart = true; }

void IOT::_restart(void)
{
    console.printf(LOG::TITLE, "** System Restart **");
    xEventGroupClearBits(_eventGroup, EZIOT_BIT);
    _needRestart = false;

    for (int i = 5; i > 0; i--)
    {
        console.printf(LOG::WARNING, "Restart in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    stop();
    _reboot();
}

void IOT::_reboot(void)
{
    console.printf(LOG::TITLE, "** Rebooting **");

    fflush(stdout);
    Serial.flush();
    Serial.end();

    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_restart();
}

/*
** Controller
*/
void IOT::_control(DEVICE* device, iot_control_t mode)
{
    if (device)
    {
        do
        {
            SERVICE* service;

            if (mode == CONTROL::INIT)
            {
                _nodeCount++;

                // Alexa MUST have a FriendlyName set, so we make one up for now!
                if (device->upnpFriendlyName() == "")
                {
                    String fn = parseURN(device->upnpDeviceType());

                    fn += " (";
                    fn += _nodeCount;
                    fn += ")";

                    device->upnpFriendlyName(fn);
                }

                if (!device->_httpServer)
                {
                    if ((device->_homeDevice == &root) && device->_httpPort != 0 && device->_httpPort != root._httpPort)
                    {
                        ESP_LOGV(iotTag, "HTTP: Create (%u) Server: %s", device->_httpPort,
                                 device->upnpDeviceType().c_str());

                        device->_httpServer = new HTTP::SERVER();

                        if (device->_httpServer)
                        {
                            device->_httpServer->httpHandler(device);
                        }
                        else
                            console.printf(LOG::ERROR, "HTTP: Error creating server for: %s",
                                           device->upnpDeviceType().c_str());
                    }
                    else
                    {
                        ESP_LOGV(iotTag, "HTTP: Share Root %u Server: %s", root._httpPort,
                                 device->upnpDeviceType().c_str());

                        device->_httpPort = 0;
                        device->_httpServer = &root;
                        device->_httpServer->httpHandler(device);
                    }
                }
            }
            else if (mode == CONTROL::START)
            {
                if (device->_httpServer && device->_httpPort != 0)
                {
                    device->_httpSetup(device->_httpServer);
                    device->_httpServer->httpStart(device->_httpPort);
                }
                iot.console.printf(LOG::INFO1, "DEVICE: '%s' Listening on: %d", device->upnpDeviceType().c_str(),
                                   device->httpPort());
            }
            else if (mode == CONTROL::LOOP && device->_httpServer && device->_httpPort != 0)
            {
                device->_httpServer->httpLoop();
            }
            else if (mode == CONTROL::STOP && device->_httpServer && device->_httpPort != 0)
            {
                device->_httpServer->httpStop();
            }

            device->_control(mode);

            if ((service = device->_headService))
            {
                do
                {
                    if (mode == CONTROL::INIT)
                    {
                        _nodeCount++;

                        if (service->_mode != SERVICE::MODE::CONFIG && device->_httpServer)
                        {
                            device->_httpServer->httpHandler(service);
                        }

                        // Create NVS Key
                        uint32_t t0 = device->_homeDevice ? device->_homeDevice->_iotCode : 0xE3107000;
                        uint32_t t1 = foldString(device->upnpDeviceType(), t0);
                        uint32_t t2 = foldString(service->_name, t1);
                        char tag[16] = {0};

                        service->_iotCode = t2 ^ t1 ^ device->_iotCode;

                        sprintf(tag, "%2.2X-%8.8X", device->_iotCode & 0xFFFF, service->_iotCode);
                        ESP_LOGV(iotTag, "TAG: %s", tag);

                        // Open NVS for the service
                        esp_err_t err;
                        if ((err = nvs_open(tag, NVS_READWRITE, &service->_nvsHandle)))
                        {
                            service->_nvsHandle = 0;
                            ESP_LOGE(_tag, "NVS: Open failed: %s", nvs_error(err));
                        }

                        service->_initialise();
                        
                        if (service->_onActivityCb)
                            (void)service->_onActivityCb(nullptr, SERVICE::CALLBACK::INIT, service);
                    }
                    else if (mode == CONTROL::START)
                    {
                        if (service->_onActivityCb)
                            (void)service->_onActivityCb(nullptr, SERVICE::CALLBACK::START, service);
                    }
                    else if (mode == CONTROL::STOP)
                    {
                        if (service->_onActivityCb)
                            (void)service->_onActivityCb(nullptr, SERVICE::CALLBACK::STOP, service);

                        if (service->_nvsHandle)
                        {
                            nvs_close(service->_nvsHandle);
                            service->_nvsHandle = 0;
                        }
                    }
                    else if (mode == CONTROL::LOOP)
                    {
                        if (service->_onActivityCb)
                            (void)service->_onActivityCb(nullptr, SERVICE::CALLBACK::LOOP, service);
                    }

                } while ((service = service->_nextService));
            }

            _control(device->_headDevice, mode);
        } while ((device = device->_nextDevice));
    }
}
