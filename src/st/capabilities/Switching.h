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
#if !defined(_ST_SWITCHING_H)
#define _ST_SWITCHING_H
#include "core/upnp_scp.h"

namespace EZ
{
    namespace ST
    {
        class SWITCHING : public UPNP::SCP
        {
        public:
            UPNP::ACTION SetSwitch;
            UPNP::ACTION GetSwitch;
            VAR::BOOLEAN SwitchState;

            SWITCHING(uint16_t pin, const char *id = "Switching.1")
                : SCP("Switching:1", id),
                  SetSwitch("SetSwitch", &SwitchState, "newSwitchState", false, true),
                  GetSwitch("GetSwitch", &SwitchState, "SwitchState", true), SwitchState("Switch", true, false, false),
                  _userActivityCb(nullptr), _pin(pin)
            {
                _upnpXMLNS = EZ_UPNP_SCHEMA_SERVICE_XMLNS;  // Need this for standard UPnP tools to work!

                _onActivityCb = std::bind(&SWITCHING::_SwitchActivity, this, std::placeholders::_1,
                                          std::placeholders::_2, std::placeholders::_3);

                addActivity(SetSwitch);
                addActivity(GetSwitch);
                addActivity(SwitchState);
            }

            void onActivity(onActivityCb cb) { _userActivityCb = cb; }

        protected:
            onActivityCb _userActivityCb;
            uint16_t _pin;

            bool _SwitchActivity(ACTIVITY* activity, CALLBACK type, void* vp)
            {
                bool result = true;

                if (_userActivityCb)
                    result = _userActivityCb(activity, type, vp);

                if (_pin != NOT_A_PIN && result)
                {
                    // On Initialisation, we setup the digital pin and set its state from
                    // the state variable in case we are saving last state to NVS
                    //
                    if (vp == this && type == CALLBACK::INIT)
                    {
                        // Set pin to OUTPUT
                        pinMode(_pin, OUTPUT);

                        // This will match the next condition so we set start-up state
                        activity = &SwitchState;
                        type = CALLBACK::POST_CHANGE;
                    }

                    if (activity == &SwitchState && type == CALLBACK::POST_CHANGE)
                    {
                        digitalWrite(_pin, SwitchState.native());
                    }

                    if (type == CALLBACK::LOOP)
                    {
                        //Serial.printf("SwitchState: (%d)\n", digitalRead(_pin));
                        SwitchState.native((bool)digitalRead(_pin));
                    }
                }

                return result;
            }
        };
    } // namespace ST
} // namespace EZ
#endif // _ST_SWITCHING_H
