/*
** EZIoT - UPNP Dimmable Light Device
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
#if !defined(_UPNP_DIMMABLELIGHT_H)
#define _UPNP_DIMMABLELIGHT_H
#include "Dimming.h"
#include "SwitchPower.h"

namespace EZ
{
    namespace UPNP
    {
        class DIMMABLELIGHT : public DEVICE
        {
        public:
            SWITCHPOWER SwitchPower;
            DIMMING Dimming;

            DIMMABLELIGHT() : DIMMABLELIGHT(nullptr, 0) {}
            DIMMABLELIGHT(int port) : DIMMABLELIGHT(nullptr, port) {}
            DIMMABLELIGHT(SERVICE::onActivityCb cb, uint16_t port = 0)
                : DEVICE(port), SwitchPower(), Dimming()
            {
                _upnpXMLNS = EZ_UPNP_SCHEMA_DEVICE_XMLNS;
                _upnpDeviceType = "urn:schemas-upnp-org:device:DimmableLight:1";
                _upnpModelName = "Dimmable Light";
                _upnpModelNumber = "1.0";
                _upnpModelDescription = "EZIoT Dimmable Light 1.0";
                _upnpModelURL = "";
                _upnpVersionMajor = 1;
                _upnpVersionMinor = 1;

                SwitchPower.onActivity(cb);
                Dimming.onActivity(cb);

                addService(SwitchPower);
                addService(Dimming);
            }
        };
    } // namespace UPNP
} // namespace EZ
#endif // _UPNP_DIMMABLELIGHT_H
