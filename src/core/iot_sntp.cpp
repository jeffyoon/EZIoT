/*
** EZIoT - IOT Controller: SNTP, Network Time Sync
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
#include <apps/sntp/sntp.h>

using namespace EZ;

/*
** Date/Time Services
*/
void IOT::timeZone(const char* tz) { _timeZone.value(tz); }

void IOT::timeServer(const char* ts) { _timeSvr1.value(ts); }

void IOT::_sntpStart(void)
{
    _sntpStop();
    sntp_init();
    _timeSync();
}

void IOT::_sntpStop(void)
{
    if (sntp_enabled())
        sntp_stop();
}

void IOT::_timeSync()
{
    static bool inSync = false;
    struct tm tm;

    if (inSync)
        return;
    inSync = true;

    xEventGroupClearBits(_eventGroup, TIMESYNC_BIT);
    configTzTime(_timeZone.value().c_str(), _timeSvr1.value().c_str(), nullptr, nullptr);
    console.printf(LOG::INFO1, "SNTP: Syncing time (TZ=%s)", _timeZone.value().c_str());

    if (getLocalTime(&tm, 2000))
    {
        char buf[60];

        strftime(buf, sizeof(buf), "%c %Z", &tm);
        console.printf(LOG::INFO1, "SNTP: Date/time is: %s", buf);
        xEventGroupSetBits(_eventGroup, TIMESYNC_BIT);
    }
    else
        console.printf(LOG::WARNING, "SNTP: Time Sync Failed!");
    inSync = false;
}
