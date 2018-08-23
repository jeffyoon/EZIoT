/*
** EZIoT - UPNP Dimming Service
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
#if !defined(_UPNP_DIMMING_H)
#define _UPNP_DIMMING_H
#include "../ez.h"


namespace EZ
{
    namespace UPNP
    {
        // ENUM List MUST be nullptr terminated!
        static const char* _onEffects[] = {"OnEffectLevel", "LastSetting", "Default", nullptr};

        class DIMMING : public SCP
        {
        public:
            UPNP::ACTION SetLoadLevelTarget;
            UPNP::ACTION GetLoadLevelTarget;
            UPNP::ACTION GetLoadLevelStatus;
            UPNP::ACTION SetOnEffectLevel;
            UPNP::ACTION SetOnEffect;
            // ACTION GetOnEffectParameters;
            UPNP::ACTION StepUp;
            UPNP::ACTION StepDown;
            UPNP::ACTION StartRampUp;
            UPNP::ACTION StartRampDown;

            VAR::UI1 LoadLevelTarget;
            VAR::UI1 LoadLevelStatus;
            VAR::UI1 OnEffectLevel;
            VAR::ENUM OnEffect;
            VAR::UI1 StepDelta;
            VAR::UI1 RampRate;
            VAR::UI4 RampTime;
            VAR::BOOLEAN IsRamping;
            VAR::BOOLEAN RampPaused;

            DIMMING()
                : SCP("urn:schemas-upnp-org:service:Dimming:1", "urn:upnp-org:serviceId:Dimming.1"),
                  SetLoadLevelTarget("SetLoadLevelTarget", &LoadLevelTarget, "newLoadLevelTarget", false),
                  GetLoadLevelTarget("GetLoadLevelTarget", &LoadLevelTarget, "retLoadLevelTarget", true, true),
                  GetLoadLevelStatus("GetLoadLevelStatus", &LoadLevelStatus, "retLoadLevelStatus", true, true),
                  SetOnEffectLevel("SetOnEffectLevel", &OnEffectLevel, "newOnEffectLevel", false),
                  SetOnEffect("SetOnEffect", &OnEffect, "newOnEffect", false),
                  // GetOnEffectParameters() - 2 Args!!?
                  StepUp("StepUp"), StepDown("StepDown"), StartRampUp("StepUp"), StartRampDown("StepDown"), 
                  
                  LoadLevelTarget("LoadLevelTarget", false, false, 0, 0, 100),
                  LoadLevelStatus("LoadLevelStatus", true, false, 0, 0, 100),
                  OnEffectLevel("OnEffectLevel", false, false, 0, 0, 100),
                  OnEffect("OnEffect", false, false, 2, &_onEffects[0]),
                  
                  // NOTE: StepDelta - default value to be defined, currently set at 10
                  StepDelta("StepDelta", true, false, 10, 1, 100), RampRate("RampRate", true, false, 0, 0, 100),
                  RampTime("RampTime", false, false, 0), IsRamping("IsRamping", true, false, false),
                  RampPaused("RampPaused", true, false, false)
            {
                _upnpXMLNS = EZ_UPNP_SCHEMA_SERVICE_XMLNS;
                _upnpVersionMajor = 1;
                _upnpVersionMinor = 1;

                addActivity(SetLoadLevelTarget);
                addActivity(GetLoadLevelTarget);
                addActivity(GetLoadLevelStatus);
                addActivity(SetOnEffectLevel);
                addActivity(SetOnEffect);
                //addActivity(GetOnEffectParameters);
                addActivity(StepUp);
                addActivity(StepDown);
                addActivity(StartRampUp);
                addActivity(StartRampDown);

                addActivity(LoadLevelTarget);
                addActivity(LoadLevelStatus);
                addActivity(OnEffectLevel);
                addActivity(OnEffect);
                addActivity(StepDelta);
                addActivity(RampRate);
                addActivity(RampTime);
                addActivity(IsRamping);
                addActivity(RampPaused);
            }
        };
    } // namespace UPNP
} // namespace EZ
#endif // _UPNP_DIMMING_H
