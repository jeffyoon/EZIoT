/*
** EZIoT - String Enumeration Variable Class
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
#if !defined(_EZI_VAR_CHAR_H)
#define _EZI_VAR_CHAR_H
#include "var_numeric.h"

namespace EZ
{
    namespace VAR
    {
        class CHAR : public NUMERIC<char>
        {
        public:
            CHAR(const char* name, bool events, bool nvm, char defVal = 0, char minVal = 0, char maxVal = UCHAR_MAX,
                 char stepVal = 0)
                : NUMERIC(name, "char", events, nvm, defVal, minVal, maxVal, stepVal)
            {
            }

            esp_err_t _loadValue(uint32_t nvsHandle)
            {
                return nvs_get_i8(nvsHandle, name().c_str(), (int8_t*)&_activeValue);
            }
            esp_err_t _saveValue(uint32_t nvsHandle) { return nvs_set_i8(nvsHandle, name().c_str(), _activeValue); }
        };

    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_CHAR_H
