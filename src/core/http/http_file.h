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
#ifndef _EZ_HTTP_FILE_HANDLER_H
#define _EZ_HTTP_FILE_HANDLER_H
#include "http_handler.h"

namespace EZ
{
    namespace HTTP
    {
        class FILE_HANDLER : public HANDLER
        {
        public:
            FILE_HANDLER(FS& fs, const char* path, const char* uri, const char* cache_header)
                : _fs(fs), _uri(uri), _path(path), _cache_header(cache_header)
            {
                _isFile = fs.exists(path);

                ESP_LOGV(iotTag, "FILE: path=%s uri=%s isFile=%d, cache_header=%s\r\n", path, uri, _isFile,
                         cache_header);

                _baseUriLength = _uri.length();
            }

        protected:
            bool _httpAccept(METHOD method, String requestUri) override
            {
                if (method != METHOD::GET)
                    return false;

                if ((_isFile && requestUri != _uri) || !requestUri.startsWith(_uri))
                    return false;

                return true;
            }

            bool _httpHandle(SERVER& server, METHOD method, String requestUri) override
            {
                if (!_httpAccept(method, requestUri))
                    return false;

                ESP_LOGD(iotTag, "FILE: request=%s _uri=%s\r\n", requestUri.c_str(), _uri.c_str());

                String path(_path);

                if (!_isFile)
                {
                    // Base URI doesn't point to a file.
                    // If a directory is requested, look for index file.
                    if (requestUri.endsWith("/"))
                        requestUri += "index.htm";

                    // Append whatever follows this URI in request to get the file path.
                    path += requestUri.substring(_baseUriLength);
                }

                ESP_LOGD(iotTag, "FILE: path=%s, isFile=%d\r\n", path.c_str(), _isFile);

                String contentType = getContentType(path);

                // look for gz file, only if the original specified path is not a gz.  So part only works to send gzip
                // via content encoding when a non compressed is asked for if you point the the path to gzip you will
                // serve the gzip as content type "application/x-gzip", not text or javascript etc...
                if (!path.endsWith(".gz") && !_fs.exists(path))
                {
                    String pathWithGz = path + ".gz";
                    if (_fs.exists(pathWithGz))
                        path += ".gz";
                }

                File f = _fs.open(path, "r");

                if (!f)
                    return false;

                if (_cache_header.length() != 0)
                    server.sendHeader("Cache-Control", _cache_header);

                server.streamFile(f, contentType);
                return true;
            }

            static String getContentType(const String& path)
            {
                if (path.endsWith(".html"))
                    return MIME_TYPE_HTML;
                else if (path.endsWith(".htm"))
                    return MIME_TYPE_HTML;
                else if (path.endsWith(".css"))
                    return MIME_TYPE_CSS;
                else if (path.endsWith(".txt"))
                    return MIME_TYPE_TEXT;
                else if (path.endsWith(".js"))
                    return MIME_TYPE_JAVA;
                else if (path.endsWith(".json"))
                    return MIME_TYPE_JSON;
                else if (path.endsWith(".png"))
                    return MIME_TYPE_PNG;
                else if (path.endsWith(".gif"))
                    return MIME_TYPE_GIF;
                else if (path.endsWith(".jpg"))
                    return MIME_TYPE_JPG;
                else if (path.endsWith(".ico"))
                    return MIME_TYPE_ICO;
                else if (path.endsWith(".svg"))
                    return MIME_TYPE_SVG;
                else if (path.endsWith(".ttf"))
                    return "application/x-font-ttf";
                else if (path.endsWith(".otf"))
                    return "application/x-font-opentype";
                else if (path.endsWith(".woff"))
                    return "application/font-woff";
                else if (path.endsWith(".woff2"))
                    return "application/font-woff2";
                else if (path.endsWith(".eot"))
                    return "application/vnd.ms-fontobject";
                else if (path.endsWith(".sfnt"))
                    return "application/font-sfnt";
                else if (path.endsWith(".xml"))
                    return MIME_TYPE_XML;
                else if (path.endsWith(".pdf"))
                    return "application/pdf";
                else if (path.endsWith(".zip"))
                    return "application/zip";
                else if (path.endsWith(".gz"))
                    return "application/x-gzip";
                else if (path.endsWith(".appcache"))
                    return "text/cache-manifest";
                return MIME_TYPE_DATA;
            }

            FS _fs;
            String _uri;
            String _path;
            String _cache_header;
            bool _isFile;
            size_t _baseUriLength;
        };
    } // namespace HTTP
} // namespace EZ
#endif // _EZ_HTTP_FILE_HANDLER_H
