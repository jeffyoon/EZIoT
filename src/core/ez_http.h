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
#if !defined(_EZ_HTTP_H)
#define _EZ_HTTP_H
#include "ez_common.h"

/*
** Equates and Defintions
*/
#define HTTP_DOWNLOAD_UNIT_SIZE 1460

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 2048
#endif

#define HTTP_MAX_DATA_WAIT 1000  // ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT 1000  // ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000  // ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 // ms to wait for the client to close the connection

#define CONTENT_LENGTH_UNKNOWN ((size_t)-1)
#define CONTENT_LENGTH_NOT_SET ((size_t)-2)

// Common Mime Types
//
#define MIME_TYPE_DATA "application/octet-stream"
#define MIME_TYPE_GZIP "application/x-gzip"
#define MIME_TYPE_TEXT "text/plain"
#define MIME_TYPE_HTML "text/html"
#define MIME_TYPE_JAVA "application/javascript"
#define MIME_TYPE_JSON "application/json"
#define MIME_TYPE_XML "text/xml"
#define MIME_TYPE_CSS "text/css"
#define MIME_TYPE_GIF "image/gif"
#define MIME_TYPE_ICO "image/x-icon"
#define MIME_TYPE_JPG "image/jpeg"
#define MIME_TYPE_SVG "image/svg+xml"
#define MIME_TYPE_PNG "image/png"

#define MIME_TYPE_JS MIME_TYPE_JAVA
#define MIME_TYPE_JPEG MIME_TYPE_JPG

// State Values
//
#define EZ_HTTP_STOPPED 0
#define EZ_HTTP_RUNNING 1
#define EZ_HTTP_ERROR 2

namespace EZ
{
    namespace HTTP
    {
        /*
        ** Forward References
        */
        class IOT;
        class SERVER;

        enum METHOD
        {
            ANY,
            GET,
            POST,
            PUT,
            PATCH,
            DELETE,
            OPTIONS,
            SUBSCRIBE,
            UNSUBSCRIBE
        };

        enum UPLOAD_STATUS
        {
            UPLOAD_FILE_START,
            UPLOAD_FILE_WRITE,
            UPLOAD_FILE_END,
            UPLOAD_FILE_ABORTED
        };

        enum CLIENT_STATUS
        {
            NONE,
            WAIT_READ,
            WAIT_CLOSE
        };

        typedef struct
        {
            UPLOAD_STATUS status;
            String filename;
            String name;
            String type;
            size_t totalSize;   // file size
            size_t currentSize; // size of data currently in buf
            uint8_t buf[HTTP_UPLOAD_BUFLEN];
        } UPLOAD;

    } // namespace HTTP
} // namespace EZ

#include "http/http_handler.h"

namespace EZ
{
    namespace HTTP
    {
        /*
        ** Simple Web Server Class
        */
        class SERVER : protected WiFiServer
        {
        public:
            typedef std::function<void(SERVER&)> callback_t;

            friend callback_t;

            SERVER();
            ~SERVER();

            void httpStart(uint16_t port = 80);
            void httpStop(void);
            void httpLoop(void);

            uint16_t httpPort(void) { return _port; }
            
            void httpHandler(HANDLER* handler);
            void httpAuthenticate(void);
            bool httpCredentials(const char* username, const char* password);

            void on(const String& uri, callback_t fn);
            void on(const String& uri, METHOD method, callback_t fn);
            void on(const String& uri, METHOD method, callback_t fn, callback_t ufn);
            void onFile(const char* uri, FS& fs, const char* path, const char* cache_header);
            void on404(callback_t fn) { _404Handler = fn; }
            void onUpload(callback_t fn) { _uploadHandler = fn; }

            String uri(void) { return _currentUri; }
            METHOD method(void) { return _currentMethod; }
            WiFiClient client(void) { return _currentClient; }
            UPLOAD& upload(void) { return _currentUpload; }

            int args(void) { return _currentArgCount; }
            bool hasArg(String name);
            String arg(String name);
            String arg(int i);
            String argName(int i);

            int headers(void) { return _headerKeysCount; }
            bool hasHeader(String name);
            void collectHeaders(const char* headerKeys[], const size_t headerKeysCount);
            String hostHeader(void) { return _hostHeader; }
            String header(String name);
            String header(int i);
            String headerName(int i);

            void sendHeader(const String& name, const String& value, bool first = false);
            void sendContent(const String& content);

            bool send(int code, const char* content_type = NULL, const String& content = String(""));
            bool send(int code, char* content_type, const String& content);
            bool send(int code, const String& content_type, const String& content);

            void setContentLength(size_t contentLength) { _contentLength = contentLength; }
            static String urlDecode(const String& text);

            template<typename T> size_t streamFile(T& file, const String& contentType)
            {
                setContentLength(file.size());

                if (String(file.name()).endsWith(".gz") && contentType != MIME_TYPE_GZIP &&
                    contentType != MIME_TYPE_DATA)
                {
                    sendHeader("Content-Encoding", "gzip");
                }

                send(200, contentType, "");
                return _currentClient.write(file);
            }

            String methodToString(int method);
            String responseCodeToString(int code);

        private:
            struct RequestArgument
            {
                String key;
                String value;
            };

            void _addRequestHandler(HANDLER* handler);
            bool _collectHeader(const char* headerName, const char* headerValue);
            void _prepareHeader(String& response, int code, const char* content_type, size_t contentLength);
            void _handleRequest(void);

            bool _parseRequest(WiFiClient& client);
            void _parseArguments(String data);
            bool _parseForm(WiFiClient& client, String boundary, uint32_t len);
            bool _parseFormUploadAborted();
            void _uploadWriteByte(uint8_t b);
            uint8_t _uploadReadByte(WiFiClient& client);

            HANDLER* _currentHandler;
            HANDLER* _firstHandler;
            HANDLER* _lastHandler;
            callback_t _404Handler;
            callback_t _uploadHandler;

            WiFiClient _currentClient;
            METHOD _currentMethod;
            String _currentUri;
            uint8_t _currentVersion;
            CLIENT_STATUS _currentStatus;
            unsigned long _statusChange;

            int _currentArgCount;
            RequestArgument* _currentArgs;
            UPLOAD _currentUpload;

            int _headerKeysCount;
            RequestArgument* _currentHeaders;
            size_t _contentLength;
            String _responseHeaders;

            String _hostHeader;
            bool _chunked;

            uint8_t _state;
            uint16_t _port;
        };
    } // namespace HTTP
} // namespace EZ
#endif // _EZ_HTTP_H
