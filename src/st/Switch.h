/*
** EZIoT - UPNP Binary Light Device
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
#if !defined(_ST_SWITCH_H)
#define _ST_SWITCH_H
#include "capabilities/Switching.h"

namespace EZ
{
    namespace ST
    {
        class SWITCH : public DEVICE
        {
        public:
            SWITCHING SwitchMode;

            SWITCH(uint16_t pin) : SWITCH(pin, nullptr) {}
            SWITCH(uint16_t pin, SERVICE::onActivityCb cb) : DEVICE(0), SwitchMode(pin)
            {
                _upnpDeviceType = "Switch:1";
                _upnpModelName = "Switch";
                _upnpModelNumber = "1.0";
                _upnpModelDescription = "EZIoT Simple Switch 1.0";
                _upnpModelURL = "";
                _upnpFriendlyName.value("Switch (" + String(pin) + ")");
                
                SwitchMode.onActivity(cb);
                addService(SwitchMode);
            }
        };
    } // namespace ST
} // namespace EZ
#endif // _ST_SWITCH_H
