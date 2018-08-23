/*
** EZIoT - (HTTP) Really Simple Web Server Class
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
#ifndef _EZ_HTTP_HANDLER_H
#define _EZ_HTTP_HANDLER_H

namespace EZ
{
    namespace HTTP
    {
        /*
         ** HTTP Handler (Abstract) Class
         */
        class HANDLER
        {
            friend class SERVER;

        public:
            virtual ~HANDLER() {}

            // _httpCanHandle - httpAccept
            // httpHandle
            // httpUploadable
            // httpUpload

        protected:
            virtual bool _httpAccept(METHOD method, String uri)
            {
                (void)method;
                (void)uri;
                return false;
            }

            virtual bool _httpHandle(SERVER& server, METHOD method, String uri)
            {
                (void)server;
                (void)method;
                (void)uri;
                return false;
            }

            virtual bool _httpUploadable(String uri)
            {
                (void)uri;
                return false;
            }

            virtual void _httpUpload(SERVER& server, String uri, UPLOAD& upload)
            {
                (void)server;
                (void)uri;
                (void)upload;
            }

            HANDLER* _nextHandler() { return _httpNext; }
            void _nextHandler(HANDLER* r) { _httpNext = r; }

        private:
            HANDLER* _httpNext = nullptr;
        };

    } // namespace HTTP
} // namespace EZ
#endif // _EZ_HTTP_HANDLER_H
