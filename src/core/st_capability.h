/*
** EZIoT - SmartThings 'Capability' Class
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
#if !defined(_ST_CAPABILITY_H)
#define _ST_CAPABILITY_H
#include "upnp_scp.h"
#include "st_thing.h"
#include "ez_variable.h"
#include "upnp_action.h"

namespace EZ
{
    namespace ST
    {
        class CAPABILITY : public UPNP::SCP
        {

        };
    }
}
#endif // _ST_CAPABILITY_H