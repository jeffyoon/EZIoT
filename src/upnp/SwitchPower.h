/*
** EZIoT - UPNP Switch Power Service
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
#if !defined(_UPNP_SWITCHPOWER_H)
#define _UPNP_SWITCHPOWER_H
#include "../ez.h"

namespace EZ
{
    namespace UPNP
    {
        class SWITCHPOWER : public SCP
        {
        public:
            UPNP::ACTION SetTarget;
            UPNP::ACTION GetTarget;
            UPNP::ACTION GetStatus;
            VAR::BOOLEAN Target;
            VAR::BOOLEAN Status;

            SWITCHPOWER()
                : SCP("urn:schemas-upnp-org:service:SwitchPower:1", "urn:upnp-org:serviceId:SwitchPower.1"),
                  SetTarget("SetTarget", &Target, "newTargetValue", false),
                  GetTarget("GetTarget", &Target, "RetTargetValue", true),
                  GetStatus("GetStatus", &Status, "ResultStatus", true), Target("Target", false, false, false),
                  Status("Status", true, false, false)
            {
                _upnpXMLNS = EZ_UPNP_SCHEMA_SERVICE_XMLNS;
                _upnpVersionMajor = 1;
                _upnpVersionMinor = 1;

                addActivity(SetTarget);
                addActivity(GetTarget);
                addActivity(GetStatus);
                addActivity(Target);
                addActivity(Status);
            }
        };
    } // namespace UPNP
} // namespace EZ
#endif // _UPNP_SWITCHPOWER_H
