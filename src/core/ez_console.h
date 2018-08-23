/*
** EZIoT - Main Controller Class
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
#if !defined(_EZ_CONSOLE_H)
#define _EZ_CONSOLE_H
#include "ez_common.h"

namespace EZ
{
    enum class LOG
    {
        TITLE,
        INFO1,
        INFO2,
        ERROR,
        WARNING,
        INFO = INFO1
    };

    class CONSOLE
    {
    public:
        CONSOLE(Print& printer = Serial) : CONSOLE(&printer) {}
        CONSOLE(Print* printer = &Serial) : _printer(printer) {}
        virtual ~CONSOLE() {}

        size_t printf(LOG type, const char* format, ...)
        {
            if (!_printer)
                return 0;

            char loc_buf[64];
            char* temp = loc_buf;
            va_list arg;
            va_list copy;
            va_start(arg, format);
            va_copy(copy, arg);
            size_t len = vsnprintf(NULL, 0, format, arg);
            va_end(copy);

            if (len >= sizeof(loc_buf))
            {
                temp = new char[len + 1];
                if (temp == NULL)
                {
                    return 0;
                }
            }

            _log(type);

            len = vsnprintf(temp, len + 1, format, arg);
            _printer->write((uint8_t*)temp, len);

            va_end(arg);

            if (len > 64)
            {
                delete[] temp;
            }

            len += _printer->print("\r\n");

            return len;
        }

    protected:
        void virtual _log(LOG log)
        {
            switch (log)
            {
                case LOG::ERROR:
                    _printer->printf("[E] ");
                    break;
                case LOG::WARNING:
                    _printer->printf("[W] ");
                    break;
                case LOG::TITLE:
                case LOG::INFO1:
                case LOG::INFO2:
                default:
                    //_printer->printf("[I] ");
                    break;
            }
        }

    private:
        Print* _printer;
    };
} // namespace EZ
#endif // _EZ_CONSOLE_H
