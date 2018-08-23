/*
** EZIoT - WEMO (UPNP) Socket Device
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
#if !defined(_EZI_WEMO_SOCKET_H)
#define _EZI_WEMO_SOCKET_H
#include "../ez.h"

namespace EZ
{
    namespace WEMO
    {
        static const char* _binaryState = "BinaryState";

        class BASICEVENT : public UPNP::SCP
        {
        public:
            UPNP::ACTION SetBinaryState;
            UPNP::ACTION GetBinaryState;
            VAR::BOOLEAN BinaryState;

            BASICEVENT()
                : UPNP::SCP("urn:Belkin:service:basicevent:1", "urn:Belkin:serviceId:basicevent1"),
                  SetBinaryState("SetBinaryState", &BinaryState, _binaryState, false, true),
                  GetBinaryState("GetBinaryState", &BinaryState, _binaryState, true, true),
                  BinaryState(_binaryState, true, false, false)
            {
                _upnpXMLNS = "urn:Belkin:service-1-0";
                _upnpVersionMajor = 1;
                _upnpVersionMinor = 0;

                addActivity(SetBinaryState);
                addActivity(GetBinaryState);
                addActivity(BinaryState);
            }

            String urlSchema(bool pathOnly = false) { return "/eventservice.xml"; }
            String urlControl(bool pathOnly = false) { return "/upnp/control/basicevent1"; }
            String urlEvents(bool pathOnly = false) { return "/upnp/event/basicevent1"; }
        };

        class SOCKET : public DEVICE
        {
        public:
            BASICEVENT BasicEvent;

            SOCKET() : SOCKET(nullptr, 0) {}
            SOCKET(int port) : SOCKET(nullptr, port) {}
            SOCKET(SERVICE::onActivityCb cb, uint16_t port = 0) : DEVICE(port), BasicEvent()
            {
                _upnpXMLNS = "urn:Belkin:device-1-0";
                _upnpDeviceType = "urn:Belkin:device:controllee:1";
                _upnpManufacturer = "Belkin International Inc.";
                _upnpManufacturerURL = "http://www.belkin.com";
                _upnpModelName = "EZIoT Emulated Socket";
                _upnpModelNumber = "0.1"; //"3.1415";
                _upnpModelDescription = "Belkin Plugin Socket 1.0";
                _upnpModelURL = "http://www.belkin.com/plugin/";
                _upnpVersionMajor = 1;
                _upnpVersionMajor = 0;

                BasicEvent.onActivity(cb);

                addService(BasicEvent);
            }

            String urlSchema(bool pathOnly = false)
            {
                String url = pathOnly ? "" : urlBase();

                url.concat("/setup.xml");
                return url;
            }

            String upnpUUID(void)
            {
                String uuid("Socket-1_0-");
                uuid.concat(DEVICE::upnpUUID());

                return uuid;
            }

            String upnpExtra(void) { return xmlTag("binaryState", BasicEvent.BinaryState.value(), false); }
        };
    } // namespace WEMO
} // namespace EZ
#endif // _EZI_WEMO_SOCKET_H
/******************************************************************************/
