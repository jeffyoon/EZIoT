/*
** EZIoT - Really Simple (HTTP) Web Server Class
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** This is based upon the ESP8266WebServer library,
** Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZI_PAGE_HANDLER_H
#define _EZI_PAGE_HANDLER_H
#include "http_handler.h"

namespace EZ
{
    namespace HTTP
    {
        class PAGE_HANDLER : public HANDLER
        {
        public:
            PAGE_HANDLER(SERVER::callback_t fn, SERVER::callback_t ufn, const String& uri, METHOD method)
                : _fn(fn), _ufn(ufn), _method(method), _uri(uri)
            {
            }

        protected:
                    bool _httpAccept(METHOD method, String uri) override
            {
                if (_method != METHOD::ANY && _method != method)
                    return false;

                if (uri != _uri)
                    return false;

                return true;
            }

            bool _httpUploadable(String uri) override
            {
                if (!_ufn || !_httpAccept(METHOD::POST, uri))
                    return false;

                return true;
            }

            bool _httpHandle(SERVER& server, METHOD method, String uri) override
            {
                (void)server;
                if (!_httpAccept(method, uri))
                    return false;

                _fn(server);
                return true;
            }

            void _httpUpload(SERVER& server, String uri, UPLOAD& upload) override
            {
                (void)server;
                (void)upload;
                if (_httpUploadable(uri))
                    _ufn(server);
            }

            SERVER::callback_t _fn;
            SERVER::callback_t _ufn;
            METHOD _method;
            String _uri;
        };
    } // namespace HTTP
} // namespace EZ
#endif // _EZ_HTTP_PAGE_HANDLER_H
