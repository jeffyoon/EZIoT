/*
** EZIoT - IOT Controller: SSDP
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
#include "iot.h"
#include "tool/ez_timer.h"
#include "upnp_scp.h"

using namespace EZ;

/*
** HTTP/1.1 200 OK
** DATE: when response was generated
** EXT: Required for backwards compatibility with UPnP 1.0. (Header field name only; no field value.)
** ST: search target
** CACHE-CONTROL: max-age = seconds until advertisement expires
** LOCATION: URL for UPnP description for root device
** SERVER: OS/version UPnP/2.0 product/version
** USN: composite identifier for the advertisement
** BOOTID.UPNP.ORG: number increased each time device sends an initial announce or an update message
** CONFIGID.UPNP.ORG: number used for caching description information
** SEARCHPORT.UPNP.ORG: number identifies port on which device responds to unicast M-SEARCH
*/
static const char _ssdp_respond_template[] = "HTTP/1.1 200 OK\r\n"
                                             "DATE: %s\r\n"
                                             "EXT:\r\n"
                                             "ST: %s\r\n";
// x

/*
** NOTIFY * HTTP/1.1
** HOST: 239.255.255.250:1900
** NT: notification type
** NTS: ssdp:alive
** CACHE-CONTROL: max-age = seconds until advertisement expires
** LOCATION: URL for UPnP description for root device
** SERVER: OS/version UPnP/2.0 product/version
** USN: composite identifier for the advertisement
** BOOTID.UPNP.ORG: number increased each time device sends an initial announce or an update message
** CONFIGID.UPNP.ORG: number used for caching description information
** SEARCHPORT.UPNP.ORG: number identifies port on which device responds to unicast M-SEARCH
*/
static const char _ssdp_notify_template[] = "NOTIFY * HTTP/1.1\r\n"
                                            "HOST: 239.255.255.250:1900\r\n"
                                            "NT: %s\r\n"
                                            "NTS: ssdp:%s\r\n";

static const char _ssdp_packet_template[] = "CACHE-CONTROL: max-age=%u\r\n"
                                            "LOCATION: %s\r\n"
                                            "SERVER: %s\r\n"
                                            "USN: %s\r\n"
                                            //"OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
                                            //"01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
                                            //"X-User-Agent: redsonic\r\n"
                                            "%s" // Device specific headers ssdpExtra()!
                                            "\r\n";

/*
** SSDP Advertiser Task
*/
static volatile TaskHandle_t _ssdp_handle = NULL;

void IOT::_ssdpTask(void* pv)
{
    /*
     ** Devices should wait a random interval (e.g. between 0 and 100milliseconds) before sending an initial
     ** set of advertisements in order to reduce the likelihood of network storms; this random interval should
     ** also be applied on occasions where the device obtains a new IP address or a new UPnP-enabled interface
     ** is installed.
     **
     ** Due to the unreliable nature of UDP, devices should send the entire set of discovery messages more than
     ** once with some delay between sets e.g. a few hundred milliseconds. To avoid network congestion discovery
     ** messages should not be sent more than three times. In addition, the device shall re-send its
     ** advertisements periodically prior to expiration of the duration specified in the CACHE-CONTROL header
     ** field; it is Recommended that such refreshing of advertisements be done at a randomly-distributed
     ** interval of less than one-half of the advertisement expiration time
     */
    TIMER nextAdvert(random(5, 9) * 100);
    int bootCount = 3 - 1;
    EventBits_t bits;

    iot._ssdpAdvertAge = EZ_SSDP_ADVERT_AGE; // CACHE-CONTROL max-age

    while ((bits = xEventGroupGetBits(iot._eventGroup)) & iot.SSDP_BIT)
    {
        vTaskDelay(portTICK_PERIOD_MS);

        if (nextAdvert.timerExpired())
        {
            if (bits & iot.CONNECTED_BIT)
            {
                // On bootup, we send adverts x (3) times ~200-500ms apart
                if (bootCount)
                {
                    nextAdvert = random(2, 5) * 100;
                    bootCount--;
                }
                else // Approx ~50% of CACHE-CONTROL age, reissue adverts
                    nextAdvert.timerPeriod(random(1000L, iot._ssdpAdvertAge * 600));
                iot._ssdpAdvertise(iot._headDevice, SSDP::ALIVE);
                iot.console.printf(LOG::INFO2, "SSDP: Advert in %d ms", nextAdvert.timerPeriod());
            }
            else
            {
                nextAdvert.timerPeriod(1000); // No WiFi! - wait a second before retry
                iot.console.printf(LOG::WARNING, "SSDP: WiFi not connected, retry advert in %d ms", nextAdvert);
            }
            nextAdvert.timerReset();
        }
    }

    iot._ssdpAdvertise(iot._headDevice, SSDP::BYEBYE);
    iot._ssdpUDP.close();
    _ssdp_handle = NULL;
    vTaskDelete(NULL);
    iot.console.printf(LOG::INFO1, "SSDP: Stopped.");
}

/*
** SSDP Adverts
*/
void IOT::_ssdpAdvertise(DEVICE* device, ssdp_method_t method)
{
    if (!device)
        return;
    do
    {
        SERVICE* service;

        _ssdpNotify(device, method);

        if ((service = device->headService()))
        {
            do
            {
                if (service->mode() == SERVICE::MODE::UPNP)
                {
                    _ssdpNotify(reinterpret_cast<UPNP::SCP*>(service), method);
                }
            } while ((service = service->nextService()));
        }

        _ssdpAdvertise(device->headDevice(), method);
        vTaskDelay(10);
    } while ((device = device->nextDevice()));
}

/*
** SSDP Control
*/
void IOT::_ssdpStart(void)
{
    if (!_ssdp_handle)
    {
        xEventGroupSetBits(_eventGroup, SSDP_BIT);
        xTaskCreate(_ssdpTask, "iotSSDP", 4096, NULL, tskIDLE_PRIORITY, (TaskHandle_t*)&_ssdp_handle);

        if (!_ssdp_handle)
        {
            xEventGroupClearBits(_eventGroup, SSDP_BIT);
            console.printf(LOG::ERROR, "SSDP: failed to create advertiser task.");
        }

        if (_ssdpUDP.listenMulticast(EZ_SSDP_MULTICAST_ADDR, EZ_SSDP_MULTICAST_PORT, EZ_SSDP_MULTICAST_TTL))
        {
            console.printf(LOG::INFO1, "SSDP: Listening on port: %d", EZ_SSDP_MULTICAST_PORT);
            _ssdpUDP.onPacket(std::bind(&IOT::_ssdpRequest, this, std::placeholders::_1));
        }
    }
}

void IOT::_ssdpStop(void)
{
    xEventGroupClearBits(_eventGroup, SSDP_BIT);

    if (_ssdp_handle)
    {
        // Hmmm, maybe want to think about setting a timer
        // and killing the task if it expires and not stopped
        // gracefully!
        //
        while (_ssdp_handle)
        {
            vTaskDelay(10);
        }
    }
}

void IOT::_ssdpNotify(DEVICE* device, ssdp_method_t method, AsyncUDPPacket* packet)
{
    if ((device) && device->ssdpAlive())
    {
        String uu = device->upnpUDN();

        if (device->_rootDevice == &root)
        {
            String nt("upnp:rootdevice");
            String usn = uu + "::" + nt;

            _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), nt.c_str(), device->ssdpExtra().c_str(),
                         device->upnpServer().c_str(), method, packet);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        _ssdpRespond(device->urlSchema(false).c_str(), uu.c_str(), uu.c_str(), device->ssdpExtra().c_str(),
                     device->upnpServer().c_str(), method, packet);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        String nt = device->upnpDeviceType();
        String usn = uu + "::" + nt;
        _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), nt.c_str(), device->ssdpExtra().c_str(),
                     device->upnpServer().c_str(), method, packet);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void IOT::_ssdpNotify(UPNP::SCP* service, ssdp_method_t method, AsyncUDPPacket* packet)
{
    if (service)
    {
        DEVICE* device = service->baseDevice();

        if ((device) && device->ssdpAlive() && service->mode() == SERVICE::MODE::UPNP)
        {
            String nt = service->upnpServiceType();
            String usn = device->upnpUDN() + "::" + nt;

            _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), nt.c_str(), device->ssdpExtra().c_str(),
                         device->upnpServer().c_str(), method, packet);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

void IOT::_ssdpRespond(const char* loc, const char* usn, const char* stnt, const char* dx, const char* mv,
                       ssdp_method_t method, AsyncUDPPacket* packet)
{
    char buffer[512];
    int len;

    if (!dx)
        dx = "";

    if (method != SSDP::NONE)
    {
        const char* nts = (method == SSDP::BYEBYE ? "byebye" : method == SSDP::UPDATE ? "update" : "alive");

        len = snprintf(buffer, sizeof(buffer), _ssdp_notify_template, stnt, nts);

        console.printf(LOG::INFO2, "SSDP: %s - %s", nts, usn);
    }
    else
    {
        struct tm info;
        time_t tick;

        time(&tick);
        localtime_r(&tick, &info);

        len = snprintf(buffer, sizeof(buffer), _ssdp_respond_template, dateRFC1123(&info).c_str(), stnt);
    }

    snprintf(&buffer[len], sizeof(buffer) - len, _ssdp_packet_template, _ssdpAdvertAge, loc, mv, usn, dx);

    if (method != SSDP::NONE && !packet)
    {
        _ssdpUDP.writeTo((const uint8_t*)&buffer[0], strlen(buffer), EZ_SSDP_MULTICAST_ADDR, EZ_SSDP_MULTICAST_PORT);
        //_ssdpUDP.print(buffer);
    }
    else if (packet)
    {
        console.printf(LOG::INFO2, "SSDP: Hello (%s:%u) - %s [%s]", packet->remoteIP().toString().c_str(),
                       packet->remotePort(), stnt, usn);
        _ssdpUDP.writeTo((const uint8_t*)&buffer[0], strlen(buffer), packet->remoteIP(), packet->remotePort());
    }

    // ESP_LOGV(iotTag, "\n%s", buffer);
}

/*
** M-SEARCH * HTTP/1.1
** HOST: 239.255.255.250:1900
** MAN: "ssdp:discover"
** MX: seconds to delay response
** ST: search target
** USER-AGENT: OS/version UPnP/2.0 product/version
** CPFN.UPNP.ORG: friendly name of the control point
** CPUUID.UPNP.ORG: uuid of the control point
*/
void IOT::_ssdpRequest(AsyncUDPPacket packet)
{
    typedef enum
    {
        START,
        MAN,
        ST,
        MX,
        UNKNOWN
    } headers;

    int _mx = 0;
    String token;
    String _st = "";
    String _man = "";
    headers header = START;

    if (!(xEventGroupGetBits(_eventGroup) & EZIOT_BIT))
    {
        ESP_LOGW(iotTag, "SSDP: Packet Ignored, Services not ready!");
        packet.flush();
        return;
    }

    // ESP_LOGD(iotTag, "-=*[%s]*=-", packet.data());

    // get message type
    int res = _ssdpParse(&token, true, false, packet);
    if ((res <= 0) || token != "M-SEARCH")
    {
        // ESP_LOGV(iotTag, "SSDP: %s, Ignored", token.c_str());
        goto bailOut;
    }

    // Get URI
    res = _ssdpParse(&token, true, false, packet);
    if ((res <= 0) || token != "*")
    {
        // ESP_LOGV(iotTag, "SSDP: %s, URI Rejected", token.c_str());
        goto bailOut;
    }

    // Eat protocol (HTTP/1.1)
    res = _ssdpParse(NULL, false, false, packet);
    if (res <= 0)
    {
        goto bailOut;
    }

    while (packet.available() > 0)
    {
        res = _ssdpParse(&token, header == START, header == START, packet);

        if (res < 0 && header == START)
        {
            break;
        }

        switch (header)
        {
            case START:
                if (token.equalsIgnoreCase("MAN"))
                    header = MAN;
                else if (token.equalsIgnoreCase("ST"))
                    header = ST;
                else if (token.equalsIgnoreCase("MX"))
                    header = MX;
                else
                {
                    header = UNKNOWN;
                    // ESP_LOGV(iotTag, "UNKNOWN HEADER: %s", token.c_str());
                }
                break;

            case MAN:
                if (token != "\"ssdp:discover\"")
                {
                    ESP_LOGV(iotTag, "SSDP: MAN: %s Rejected", token.c_str());
                    goto bailOut;
                }
                _man = token;
                header = START;
                break;

            case ST:
                _st = token;
                header = START;
                break;

            case MX:
                _mx = atoi(token.c_str());
                header = START;
                break;

            case UNKNOWN:
                header = START;
                break;
        }
    }

    if (header == START)
    {
        // Limit response delay to max 6 Seconds
        // Amazon Alexa only waits a short time
        //
        uint32_t _delay = min(random(500, _mx * 1000L), 5000L);

        vTaskDelay(_delay / portTICK_PERIOD_MS);

        if (!EZ_IOT_MUTEX_TAKE())
        {
            // No mutex lock, silently fail
            packet.flush();
            return;
        }

        /*
                ESP_LOGD(iotTag, "SSDP: %s(%s:%u): %s, %s (MX=%d [%u])",
                         packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast",
                         packet.remoteIP().toString().c_str(), packet.remotePort(), _man.c_str(), _st.c_str(), _mx,
           _delay);
        */

        if (_st.equalsIgnoreCase("upnp:rootdevice"))
        {
            _ssdpRoot(packet);
        }
        else if (_st.equalsIgnoreCase("ssdp:all"))
        {
            _ssdpAll(_headDevice, packet);
        }
        else
        {
            _ssdpMatch(_headDevice, _st, packet);
        }

        EZ_IOT_MUTEX_GIVE();
        return;
    }

bailOut:
    // something broke during parsing of the message
    // while (_ssdpParse(NULL, true, true, packet) > 0);
    packet.flush();
}

void IOT::_ssdpRoot(AsyncUDPPacket& packet)
{
    DEVICE* device = _headDevice;

    if (!device)
        return;
    do
    {
        if (device->_rootDevice == &root && device->ssdpAlive())
        {
            String usn = device->upnpUDN() + "::" + device->upnpDeviceType();
            _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), "upnp:rootdevice", device->ssdpExtra().c_str(),
                         device->upnpServer().c_str(), SSDP::NONE, &packet);
        }
    } while ((device = device->_nextDevice));
}

void IOT::_ssdpAll(DEVICE* device, AsyncUDPPacket& packet)
{
    if (!device)
        return;
    do
    {
        if (device->ssdpAlive())
        {
            SERVICE* service;

            _ssdpNotify(device, SSDP::NONE, &packet);

            if ((service = device->_headService))
            {
                do
                {
                    if (service->mode() == SERVICE::MODE::UPNP)
                        _ssdpNotify(reinterpret_cast<UPNP::SCP*>(service), SSDP::NONE, &packet);
                } while ((service = service->_nextService));
            }
        }
    } while ((device = device->_nextDevice));
}

void IOT::_ssdpMatch(DEVICE* device, String& st, AsyncUDPPacket& packet)
{
    if (!device)
        return;
    do
    {
        SERVICE* service;

        if (device->ssdpAlive())
        {
            if (device->ssdpMatch(st))
            {
                String usn = device->upnpUDN() + "::" + device->upnpDeviceType();
                _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), st.c_str(), device->ssdpExtra().c_str(),
                             device->upnpServer().c_str(), SSDP::NONE, &packet);
            }

            if ((service = device->_headService))
            {
                do
                {
                    if (service->mode() == SERVICE::MODE::UPNP)
                    {
                        UPNP::SCP* upnp = reinterpret_cast<UPNP::SCP*>(service);

                        if (upnp->upnpServiceType() == st)
                        {
                            String usn = device->upnpUDN() + "::" + upnp->upnpServiceType();
                            _ssdpRespond(device->urlSchema(false).c_str(), usn.c_str(), st.c_str(),
                                         device->ssdpExtra().c_str(), device->upnpServer().c_str(), SSDP::NONE,
                                         &packet);
                        }
                    }
                } while ((service = service->_nextService));
            }
        }
    } while ((device = device->_nextDevice));
}

// writes the next token into token if token is not NULL returns -1 on message end, otherwise returns 0
int IOT::_ssdpParse(String* token, bool break_on_space, bool break_on_colon, AsyncUDPPacket& packet)
{
    if (token)
        *token = "";
    bool token_found = false;
    int cr_found = 0;

    while (packet.available() > 0)
    {
        char next = packet.read();
        switch (next)
        {
            case '\r':
            case '\n':
                cr_found++;
                if (cr_found == 3)
                {
                    // end of message reached
                    return -1;
                }
                if (token_found)
                {
                    // end of token reached
                    return packet.available();
                }
                continue;

            case ' ':
                // only treat spaces as part of text if they're not leading
                if (!token_found)
                {
                    cr_found = 0;
                    continue;
                }
                if (!break_on_space)
                {
                    break;
                }
                cr_found = 0;

                // end of token reached
                return packet.available();

            case ':':
                // only treat colons as part of text if they're not leading
                if (!token_found)
                {
                    cr_found = 0;
                    continue;
                }
                if (!break_on_colon)
                {
                    break;
                }
                cr_found = 0;

                // end of token reached
                return packet.available();

            default:
                cr_found = 0;
                token_found = true;
                break;
        }

        if (token)
        {
            (*token) += next;
        }
    }

    return 0;
}