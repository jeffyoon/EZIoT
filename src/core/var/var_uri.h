/*
** EZIoT - URI (String) Variable Class
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
#if !defined(_EZI_VAR_URI_H)
#define _EZI_VAR_URI_H
#include "var_string.h"

namespace EZ
{
    namespace VAR
    {
        class URI : public STRING
        {
            friend class IOT;

        public:
            ~URI() {}
            URI(const char* name, bool evt, bool nvs, const char* defVal = "") : STRING(name, evt, nvs, defVal, 256)
            {
                _type = "uri";
            }

            int validate(String& value)
            {
                if (value.length() > _size)
                    return EZ_SOAP_ERROR_STRING_LENGTH;
                return EZ_SOAP_ERROR_NONE;
            }

            String defaultValue(void) { return _defaultValue; }

        protected:
            String _allowedTags(void) { return ""; }

            String _getValue(void) { return _activeValue; }

            bool _setValue(String& newValue)
            {
                if (_activeValue != newValue)
                {
                    _activeValue = newValue;
                    return true;
                }

                return false;
            }

        private:
            String _activeValue;
            String _defaultValue;
        };
    } // namespace VAR
} // namespace EZ
#endif //_EZI_VAR_URI_H
