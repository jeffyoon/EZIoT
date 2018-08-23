/*
** EZIoT - Common Includes/Equates/Defintions and Functions
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
#include "ez_common.h"

namespace EZ
{
    /*
    ** Globals
    */
    const char* nvs_errors[] = {"OTHER",         "NOT_INITIALIZED",  "NOT_FOUND",    "TYPE_MISMATCH",
                                "READ_ONLY",     "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE",
                                "REMOVE_FAILED", "KEY_TOO_LONG",     "PAGE_FULL",    "INVALID_STATE",
                                "INVALID_LENGHT"};

    /*
    ** Global Date/Time Functions
    **
    ** Example RFC3339 Date: 2013-09-05T14:58:33.102-08:00
    ** Example RFC1123 Date: Wed, 28 Apr 2010 02:31:05 GMT
    */
    String dateRFC3339(void)
    {
        time_t ticks;
        time(&ticks);
        return dateRFC3339(&ticks);
    }

    String dateRFC3339(time_t* ticks)
    {
        struct tm tm = {0};
        if (ticks)
            localtime_r(ticks, &tm);
        return dateRFC3339(&tm);
    }

    String dateRFC3339(struct tm* tm)
    {
        char buf[60] = {0};
        if (tm)
            strftime(buf, sizeof(buf), "%Y-%m-%dT%T%z", tm);
        return buf;
    }

    String dateRFC1123(void)
    {
        time_t ticks;
        time(&ticks);
        return dateRFC1123(&ticks);
    }

    String dateRFC1123(time_t* ticks)
    {
        struct tm tm = {0};
        if (ticks)
            localtime_r(ticks, &tm);
        return dateRFC1123(&tm);
    }

    String dateRFC1123(struct tm* tm)
    {
        char buf[60] = {0};

        if (tm)
            strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", tm);
        return buf;
    }

    // String xmlTag(String tag, String value, bool emptyTag) { return xmlTag(tag.c_str(), value, emptyTag); }

    String xmlTag(const char* tag, String value, bool emptyTag)
    {
        String node("");

        if (value != "" || emptyTag)
        {
            node.concat("<");
            node.concat(tag);
            node.concat(">");
            node.concat(value);
            node.concat("</");
            node.concat(tag);
            node.concat(">\r\n");
        }

        return node;
    }

    /*
    ** Parse a URN string for last colon seperated field
    ** if the last field is numeric convert colon to
    ** underscore and re-parse for last valid field
    */
    String parseURN(String urn)
    {
        int i = urn.lastIndexOf(':');

        if (urn.substring(i + 1, urn.length()).toInt() != 0)
        {
            urn[i] = '_';
            // Remove any decimals
            urn.replace(".", "");
            i = urn.lastIndexOf(':');
        }
        else
            urn.replace(".", "_");

        return urn.substring(i + 1, urn.length());
    }

    // Use folding on a string, summed 4 bytes at a time
    //
    uint32_t foldString(String s, int M)
    {
        int intLength = s.length() / 4;
        uint32_t sum = 0;
        char c[4];

        for (int j = 0; j < intLength; j++)
        {
            s.toCharArray(c, 4, j * 4);
            // char c[] = s.substring(j * 4, (j * 4) + 4).toCharArray();

            long mult = 1;

            for (int k = 0; k < 4; k++)
            {
                sum += c[k] * mult;
                mult *= 256;
            }
        }

        s.toCharArray(c, 4, intLength * 4);
        // char c[] = s.substring(intLength * 4).toCharArray();
        long mult = 1;

        for (int k = 0; k < 4; k++)
        {
            sum += c[k] * mult;
            mult *= 256;
        }

        return (abs(sum) % M);
    }

} // namespace EZ