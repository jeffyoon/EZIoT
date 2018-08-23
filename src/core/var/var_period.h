/*
** EZIoT - Date and Time Variable Class
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
#if !defined(_EZI_VAR_PERIOD_H)
#define _EZI_VAR_PERIOD_H
#include "../ez_variable.h"
#include <time.h>

namespace EZ
{
    namespace VAR
    {
        /*
        ** date		    yyyymmdd                Date in a subset of ISO 8601 format without time data.
        ** dateTime	    yyyymmddThhmmssffffff   Date in ISO 8601 format with allowed time but no time zone.
        ** dateTime.tz	yyyymmddThhmmss+|-hhmm  Date in ISO 8601 format with allowed time and allowed time zone.
        ** time		    hhmmssffffff            Time in a subset of ISO 8601 format with no date and no time zone.
        ** time.tz		hhmmss+|-hhmm           Time in a subset of ISO 8601 format with allowed time zone but no date.
        **
        ** See strptime()
        */

    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_PERIOD_H
