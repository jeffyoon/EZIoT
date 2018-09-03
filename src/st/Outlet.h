/*
** EZIoT - ST Outlet Device
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
#if !defined(_ST_OUTLET_H)
#define _ST_OUTLET_H
#include "capabilities/Switching.h"

namespace EZ
{
    namespace ST
    {
        class OUTLET : public DEVICE
        {
        public:
            SWITCHING SwitchMode;

            OUTLET(uint16_t pin) : OUTLET(pin, nullptr) {}
            OUTLET(uint16_t pin, SERVICE::onActivityCb cb) : DEVICE(0), SwitchMode(pin)
            {
                _upnpDeviceType = "Outlet:1";
                _upnpModelName = "Outlet";
                _upnpModelNumber = "1.0";
                _upnpModelDescription = "EZIoT Simple Outlet Switch 1.0";
                _upnpModelURL = "";
                _upnpFriendlyName.value("Outlet (" + String(pin) + ")");
                
                SwitchMode.onActivity(cb);
                addService(SwitchMode);
            }
        };
    } // namespace ST
} // namespace EZ
#endif // _ST_OUTLET_H
