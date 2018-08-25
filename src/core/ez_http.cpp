/*
** EZIoT - Really Simple (HTTP) Web Server Class
**
** TODO: This needs updating to reflect latest espressif/arduino-esp32 git version
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
#include "ez_http.h"
#include "http/http_file.h"
#include "http/http_page.h"
#include "iot.h"

#include <FS.h>
#include <libb64/cencode.h>

using namespace EZ;
using namespace HTTP;

static const char AUTHORIZATION_HEADER[] = "Authorization";
static const char CONTENT_TYPE_HEADER[] = "Content-Type";
static const char CONTENT_LENGTH_HEADER[] = "Content-Length";

/*
** Class Construction
*/
SERVER::SERVER()
    : WiFiServer(80), _currentHandler(0), _firstHandler(0), _lastHandler(0), _currentMethod(METHOD::ANY),
      _currentVersion(0), _currentStatus(CLIENT_STATUS::NONE), _statusChange(0), _currentArgCount(0), _currentArgs(0),
      _headerKeysCount(0), _currentHeaders(0), _contentLength(0), _chunked(false), _state(EZ_HTTP_STOPPED), _port(80)
{
}

/*
** Class Deconstruction
*/
SERVER::~SERVER()
{
    httpStop();
    if (_currentHeaders)
        delete[] _currentHeaders;
    _headerKeysCount = 0;
    HANDLER* handler = _firstHandler;

    while (handler)
    {
        HANDLER* next = handler->_nextHandler();
        delete handler;
        handler = next;
    }
    close();
}

/*
** Service Startup
*/
void SERVER::httpStart(uint16_t port)
{
    if (_state != EZ_HTTP_STOPPED)
        return;

    _port = port;

    _currentStatus = CLIENT_STATUS::NONE;

    begin(_port);

    if (!_headerKeysCount)
        collectHeaders(0, 0);

    EZ::iot.console.printf(::LOG::INFO1, "HTTP: Server (%u) started.", _port);
    _state = EZ_HTTP_RUNNING;
}

/*
** Service Shutdown
*/
void SERVER::httpStop(void)
{
    if (_state == EZ_HTTP_STOPPED)
        return;

    close();
    // stop();

    EZ::iot.console.printf(::LOG::INFO1, "HTTP: Server (%u) stopped.", _port);
    _state = EZ_HTTP_STOPPED;
}

/*
** Service Update
*/
void SERVER::httpLoop(void)
{
    if (_state != EZ_HTTP_RUNNING)
        return;

    if (_currentStatus == CLIENT_STATUS::NONE)
    {
        WiFiClient client = available();

        if (!client)
        {
            return;
        }

        ESP_LOGV(iotTag, "Client Connected: (%u) %s:%u", _port,
            client.remoteIP().toString().c_str(), client.remotePort());

        _currentClient = client;
        _currentStatus = CLIENT_STATUS::WAIT_READ;
        _statusChange = millis();
    }

    bool keepCurrentClient = false;
    bool callYield = false;

    if (_currentClient.connected())
    {
        switch (_currentStatus)
        {
            case CLIENT_STATUS::NONE:
                // No-op to avoid C++ compiler warning
                break;

            case CLIENT_STATUS::WAIT_READ:
                // Wait for data from client to become available
                if (_currentClient.available())
                {
                    if (_parseRequest(_currentClient))
                    {
                        if (_currentClient.available())
                            ESP_LOGV(iotTag, "HTTP: Still got client data!");

                        _currentClient.setTimeout(HTTP_MAX_SEND_WAIT);
                        _contentLength = CONTENT_LENGTH_NOT_SET;
                        _handleRequest();

                        if (_currentClient.connected())
                        {
                            ESP_LOGV(iotTag, "HTTP: Client still connected");
                            _currentStatus = CLIENT_STATUS::WAIT_CLOSE;
                            _statusChange = millis();
                            keepCurrentClient = true;
                        }
                    }
                }
                else
                { // !_currentClient.available()
                    if (millis() - _statusChange <= HTTP_MAX_DATA_WAIT)
                    {
                        keepCurrentClient = true;
                    }
                    callYield = true;
                }
                break;

            case CLIENT_STATUS::WAIT_CLOSE:
                // Wait for client to close the connection
                if (millis() - _statusChange <= HTTP_MAX_CLOSE_WAIT)
                {
                    keepCurrentClient = true;
                    callYield = true;
                }
        }
    }

    if (!keepCurrentClient)
    {
        _currentClient = WiFiClient();
        _currentStatus = CLIENT_STATUS::NONE;
        //_currentUpload.reset();
    }

    if (callYield)
    {
        yield();
    }
}

/*
** HTTP Handlers
*/
void SERVER::httpHandler(HANDLER* handler) { _addRequestHandler(handler); }

void SERVER::_addRequestHandler(HANDLER* handler)
{
    if (!_lastHandler)
    {
        _firstHandler = handler;
        _lastHandler = handler;
    }
    else
    {
        // TODO: Check we don't already have the handler installed!

        _lastHandler->_nextHandler(handler);
        _lastHandler = handler;
    }
}

/*
** Basic HTTP Authentication
*/
void SERVER::httpAuthenticate()
{
    sendHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
    send(401);
}

bool SERVER::httpCredentials(const char* username, const char* password)
{
    if (hasHeader(FPSTR(AUTHORIZATION_HEADER)))
    {
        String authReq = header(FPSTR(AUTHORIZATION_HEADER));

        if (authReq.startsWith(F("Basic")))
        {
            authReq = authReq.substring(6);
            authReq.trim();
            char toencodeLen = strlen(username) + strlen(password) + 1;
            char* toencode = new char[toencodeLen + 1];

            if (toencode == NULL)
            {
                authReq = String();
                return false;
            }

            char* encoded = new char[base64_encode_expected_len(toencodeLen) + 1];

            if (encoded == NULL)
            {
                authReq = String();
                delete[] toencode;
                return false;
            }

            sprintf(toencode, "%s:%s", username, password);

            if (base64_encode_chars(toencode, toencodeLen, encoded) > 0 && authReq.equals(encoded))
            {
                authReq = String();
                delete[] toencode;
                delete[] encoded;
                return true;
            }
            delete[] toencode;
            delete[] encoded;
        }
        authReq = String();
    }
    return false;
}

/*
** On Handlers
*/
void SERVER::on(const String& uri, callback_t fn) { on(uri, ANY, fn); }

void SERVER::on(const String& uri, METHOD method, callback_t fn) { on(uri, method, fn, _uploadHandler); }

void SERVER::on(const String& uri, METHOD method, callback_t fn, callback_t ufn)
{
    _addRequestHandler(new PAGE_HANDLER(fn, ufn, uri, method));
}

void SERVER::onFile(const char* uri, FS& fs, const char* path, const char* cache_header)
{
    _addRequestHandler(new FILE_HANDLER(fs, path, uri, cache_header));
}

/*
** Arguments
*/
bool SERVER::hasArg(String name)
{
    for (int i = 0; i < _currentArgCount; ++i)
    {
        if (_currentArgs[i].key == name)
            return true;
    }
    return false;
}

String SERVER::arg(String name)
{
    for (int i = 0; i < _currentArgCount; ++i)
    {
        if (_currentArgs[i].key == name)
            return _currentArgs[i].value;
    }
    return String();
}

String SERVER::arg(int i)
{
    if (i < _currentArgCount)
        return _currentArgs[i].value;
    return String();
}

String SERVER::argName(int i)
{
    if (i < _currentArgCount)
        return _currentArgs[i].key;
    return String();
}

/*
** Headers
*/
bool SERVER::hasHeader(String name)
{
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        if ((_currentHeaders[i].key.equalsIgnoreCase(name)) && (_currentHeaders[i].value.length() > 0))
            return true;
    }
    return false;
}

void SERVER::collectHeaders(const char* headerKeys[], const size_t headerKeysCount)
{
    // TODO: Merge Existing tracked headers, as the master may have added some!
    _headerKeysCount = headerKeysCount + 2;
    if (_currentHeaders)
        delete[] _currentHeaders;
    _currentHeaders = new RequestArgument[_headerKeysCount];
    _currentHeaders[0].key = FPSTR(AUTHORIZATION_HEADER);
    _currentHeaders[1].key = FPSTR(CONTENT_TYPE_HEADER);

    for (int i = 2; i < _headerKeysCount; i++)
    {
        _currentHeaders[i].key = headerKeys[i - 2];
    }
}

String SERVER::header(String name)
{
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        if (_currentHeaders[i].key.equalsIgnoreCase(name))
            return _currentHeaders[i].value;
    }
    return String();
}

String SERVER::header(int i)
{
    if (i < _headerKeysCount)
        return _currentHeaders[i].value;
    return String();
}

String SERVER::headerName(int i)
{
    if (i < _headerKeysCount)
        return _currentHeaders[i].key;
    return String();
}

void SERVER::_prepareHeader(String& response, int code, const char* content_type, size_t contentLength)
{
    response = "HTTP/1." + String(_currentVersion) + " ";
    response += String(code);
    response += " ";
    response += responseCodeToString(code);
    response += "\r\n";

    if (!content_type)
        content_type = MIME_TYPE_HTML;

    sendHeader(CONTENT_TYPE_HEADER, content_type, true);
    if (_contentLength == CONTENT_LENGTH_NOT_SET)
    {
        sendHeader(FPSTR(CONTENT_LENGTH_HEADER), String(contentLength));
    }
    else if (_contentLength != CONTENT_LENGTH_UNKNOWN)
    {
        sendHeader(FPSTR(CONTENT_LENGTH_HEADER), String(_contentLength));
    }
    else if (_contentLength == CONTENT_LENGTH_UNKNOWN && _currentVersion)
    { // HTTP/1.1 or above client
        // let's do chunked
        _chunked = true;
        sendHeader("Accept-Ranges", "none");
        sendHeader("Transfer-Encoding", "chunked");
    }

    sendHeader("Connection", "close");

    response += _responseHeaders;
    response += "\r\n";
    _responseHeaders = String();
}

/*
** Senders
*/
void SERVER::sendHeader(const String& name, const String& value, bool first)
{
    String headerLine = name;
    headerLine += ": ";
    headerLine += value;
    headerLine += "\r\n";

    if (first)
    {
        _responseHeaders = headerLine + _responseHeaders;
    }
    else
    {
        _responseHeaders += headerLine;
    }
}

void SERVER::sendContent(const String& content)
{
    const char* footer = "\r\n";
    size_t len = content.length();

    if (_chunked)
    {
        char* chunkSize = (char*)malloc(11);
        if (chunkSize)
        {
            sprintf(chunkSize, "%x%s", len, footer);
            _currentClient.write(chunkSize, strlen(chunkSize));
            free(chunkSize);
        }
    }

    _currentClient.write(content.c_str(), len);
    
    if (_chunked)
    {
        _currentClient.write(footer, 2);
    }
}

bool SERVER::send(int code, char* content_type, const String& content)
{
    return send(code, (const char*)content_type, content);
}

bool SERVER::send(int code, const String& content_type, const String& content)
{
    return send(code, (const char*)content_type.c_str(), content);
}

bool SERVER::send(int code, const char* content_type, const String& content)
{
    String header;

    // Can we asume the following?
    // if(code == 200 && content.length() == 0 && _contentLength == CONTENT_LENGTH_NOT_SET)
    //  _contentLength = CONTENT_LENGTH_UNKNOWN;

    _prepareHeader(header, code, content_type, content.length());
    _currentClient.write(header.c_str(), header.length());

    if (content.length())
        sendContent(content);

    ESP_LOGD(iotTag, "Client(%s:%d): %d %s %s:%s", _currentClient.remoteIP().toString().c_str(),
             _currentClient.remotePort(), code, responseCodeToString(code).c_str(),
             methodToString(_currentMethod).c_str(), _currentUri.c_str());

    ESP_LOGV(iotTag, "\n%s%s", header.c_str(), content.c_str());

    // Hmmmm! - Without these, it doesn't work!?????
    //
    _currentClient.flush();
    _currentClient.stop();

    return true;
}

/*
** Internal Handlers
*/
void SERVER::_handleRequest()
{
    bool handled = false;

    if (!_currentHandler)
    {
        // ESP_LOGV(iotTag, "Request handler not found: %s", _currentUri.c_str());
    }
    else
    {
        handled = _currentHandler->_httpHandle(*this, _currentMethod, _currentUri);
    }

    if (!handled)
    {
        if (_404Handler)
        {
            _404Handler(*this);
        }
        else
        {
            send(404, MIME_TYPE_TEXT, String("Not found: ") + _currentUri);
        }
    }

    _currentUri = String();
}

String SERVER::methodToString(int method)
{
    switch (method)
    {
        case METHOD::POST:
            return "POST";
        case METHOD::GET:
            return "GET";
        case METHOD::PUT:
            return "PUT";
        case METHOD::PATCH:
            return "PATCH";
        case METHOD::DELETE:
            return "DELETE";
        case METHOD::OPTIONS:
            return "OPTIONS";
        case METHOD::SUBSCRIBE:
            return "SUBSCRIBE";
        case METHOD::UNSUBSCRIBE:
            return "UNSUBSCRIBE";

        default:
            return "unknown";
    }
}

String SERVER::responseCodeToString(int code)
{
    switch (code)
    {
        case 100:
            return F("Continue");
        case 101:
            return F("Switching Protocols");
        case 200:
            return F("OK");
        case 201:
            return F("Created");
        case 202:
            return F("Accepted");
        case 203:
            return F("Non-Authoritative Information");
        case 204:
            return F("No Content");
        case 205:
            return F("Reset Content");
        case 206:
            return F("Partial Content");
        case 300:
            return F("Multiple Choices");
        case 301:
            return F("Moved Permanently");
        case 302:
            return F("Found");
        case 303:
            return F("See Other");
        case 304:
            return F("Not Modified");
        case 305:
            return F("Use Proxy");
        case 307:
            return F("Temporary Redirect");
        case 400:
            return F("Bad Request");
        case 401:
            return F("Unauthorized");
        case 402:
            return F("Payment Required");
        case 403:
            return F("Forbidden");
        case 404:
            return F("Not Found");
        case 405:
            return F("Method Not Allowed");
        case 406:
            return F("Not Acceptable");
        case 407:
            return F("Proxy Authentication Required");
        case 408:
            return F("Request Time-out");
        case 409:
            return F("Conflict");
        case 410:
            return F("Gone");
        case 411:
            return F("Length Required");
        case 412:
            return F("Precondition Failed");
        case 413:
            return F("Request Entity Too Large");
        case 414:
            return F("Request-URI Too Large");
        case 415:
            return F("Unsupported Media Type");
        case 416:
            return F("Requested range not satisfiable");
        case 417:
            return F("Expectation Failed");
        case 500:
            return F("Internal Server Error");
        case 501:
            return F("Not Implemented");
        case 502:
            return F("Bad Gateway");
        case 503:
            return F("Service Unavailable");
        case 504:
            return F("Gateway Time-out");
        case 505:
            return F("HTTP Version not supported");
        default:
            return "";
    }
}

/*
** Parsers
*/
static char* readBytesWithTimeout(WiFiClient& client, size_t maxLength, size_t& dataLength, int timeout_ms)
{
    char* buf = nullptr;
    dataLength = 0;
    while (dataLength < maxLength)
    {
        int tries = timeout_ms;
        size_t newLength;
        while (!(newLength = client.available()) && tries--)
            delay(1);

        if (!newLength)
        {
            break;
        }
        if (!buf)
        {
            buf = (char*)malloc(newLength + 1);
            if (!buf)
            {
                return nullptr;
            }
        }
        else
        {
            char* newBuf = (char*)realloc(buf, dataLength + newLength + 1);
            if (!newBuf)
            {
                free(buf);
                return nullptr;
            }
            buf = newBuf;
        }
        client.readBytes(buf + dataLength, newLength);
        dataLength += newLength;
        buf[dataLength] = '\0';
    }
    return buf;
}

bool SERVER::_parseRequest(WiFiClient& client)
{
    // Read the first line of HTTP request
    // Handle bad apps who only send LF!
    String req = client.readStringUntil('\n');
    if (req[req.length() - 1] == '\r')
    {
        req.trim();
    }

    // reset header value
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        _currentHeaders[i].value = String();
    }

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);

    if (addr_start == -1 || addr_end == -1)
    {
        ESP_LOGD(iotTag, "Invalid Request: %s", req.c_str());
        return false;
    }

    String methodStr = req.substring(0, addr_start);
    String url = req.substring(addr_start + 1, addr_end);
    String versionEnd = req.substring(addr_end + 8);
    _currentVersion = atoi(versionEnd.c_str());
    String searchStr = "";
    int hasSearch = url.indexOf('?');

    if (hasSearch != -1)
    {
        searchStr = urlDecode(url.substring(hasSearch + 1));
        url = url.substring(0, hasSearch);
    }

    _currentUri = url;
    _chunked = false;

    METHOD method = METHOD::GET;

    if (methodStr == "POST")
    {
        method = METHOD::POST;
    }
    else if (methodStr == "DELETE")
    {
        method = METHOD::DELETE;
    }
    else if (methodStr == "OPTIONS")
    {
        method = METHOD::OPTIONS;
    }
    else if (methodStr == "PUT")
    {
        method = METHOD::PUT;
    }
    else if (methodStr == "PATCH")
    {
        method = METHOD::PATCH;
    }
    else if (methodStr == "SUBSCRIBE")
    {
        method = METHOD::SUBSCRIBE;
    }
    else if (methodStr == "UNSUBSCRIBE")
    {
        method = METHOD::UNSUBSCRIBE;
    }
    else
    {
        ESP_LOGE(iotTag, "Method: %s URL: %s Search: %s", methodStr.c_str(), url.c_str(), searchStr.c_str());
    }
    _currentMethod = method;

    // Attach handler
    HANDLER* handler;

    for (handler = _firstHandler; handler; handler = handler->_nextHandler())
    {
        if (handler->_httpAccept(_currentMethod, _currentUri))
            break;
    }
    _currentHandler = handler;

    String formData;

    // Below is needed only when POST type request
    if (method == METHOD::POST || method == METHOD::PUT || method == METHOD::PATCH || method == METHOD::DELETE)
    {
        String boundaryStr;
        String headerName;
        String headerValue;
        bool isForm = false;
        bool isEncoded = false;
        uint32_t contentLength = 0;

        // Parse headers
        while (1)
        {
            // Handle bad apps who only send LF!
            String req = client.readStringUntil('\n');
            if (req[req.length() - 1] == '\r')
            {
                req.trim();
            }

            if (req == "")
                break; // no more headers

            int headerDiv = req.indexOf(':');

            if (headerDiv == -1)
            {
                break;
            }

            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 1);
            headerValue.trim();
            _collectHeader(headerName.c_str(), headerValue.c_str());

            // ESP_LOGV(iotTag, "Header: %s = %s", headerName.c_str(), headerValue.c_str());

            if (headerName.equalsIgnoreCase(CONTENT_TYPE_HEADER))
            {
                if (headerValue.startsWith("text/plain"))
                {
                    isForm = false;
                }
                else if (headerValue.startsWith("application/x-www-form-urlencoded"))
                {
                    isForm = false;
                    isEncoded = true;
                }
                else if (headerValue.startsWith("multipart/"))
                {
                    boundaryStr = headerValue.substring(headerValue.indexOf('=') + 1);
                    isForm = true;
                }
            }
            else if (headerName.equalsIgnoreCase(CONTENT_LENGTH_HEADER))
            {
                contentLength = headerValue.toInt();
            }
            else if (headerName.equalsIgnoreCase("Host"))
            {
                _hostHeader = headerValue;
            }
        }

        if (!isForm)
        {
            size_t plainLength;
            char* plainBuf = readBytesWithTimeout(client, contentLength, plainLength, HTTP_MAX_POST_WAIT);

            if (plainLength < contentLength)
            {
                free(plainBuf);
                return false;
            }

            if (contentLength > 0)
            {
                if (searchStr != "")
                    searchStr += '&';
                if (isEncoded)
                {
                    // url encoded form
                    String decoded = urlDecode(plainBuf);
                    size_t decodedLen = decoded.length();
                    memcpy(plainBuf, decoded.c_str(), decodedLen);
                    plainBuf[decodedLen] = 0;
                    searchStr += plainBuf;
                }

                _parseArguments(searchStr);

                if (!isEncoded)
                {
                    // plain post json or other data
                    RequestArgument& arg = _currentArgs[_currentArgCount++];
                    arg.key = "plain";
                    arg.value = String(plainBuf);
                }

                // ESP_LOGV(iotTag, "Plain: %s", plainBuf);

                free(plainBuf);
            }
        }

        if (isForm)
        {
            _parseArguments(searchStr);
            if (!_parseForm(client, boundaryStr, contentLength))
            {
                return false;
            }
        }
    }
    else
    {
        String headerName;
        String headerValue;

        // Parse headers
        while (1)
        {
            // Handle bad apps who only send LF!
            String req = client.readStringUntil('\n');
            if (req[req.length() - 1] == '\r')
            {
                req.trim();
            }

            if (req == "")
                break; // no moar headers

            int headerDiv = req.indexOf(':');
            if (headerDiv == -1)
            {
                break;
            }

            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 2);
            _collectHeader(headerName.c_str(), headerValue.c_str());

            if (headerName.equalsIgnoreCase("Host"))
            {
                _hostHeader = headerValue;
            }
        }
        _parseArguments(searchStr);
    }

    return true;
}

bool SERVER::_collectHeader(const char* headerName, const char* headerValue)
{
    for (int i = 0; i < _headerKeysCount; i++)
    {
        if (_currentHeaders[i].key.equalsIgnoreCase(headerName))
        {
            _currentHeaders[i].value = headerValue;
            return true;
        }
    }
    return false;
}

void SERVER::_parseArguments(String data)
{
    if (_currentArgs)
        delete[] _currentArgs;
    _currentArgs = 0;

    if (data.length() == 0)
    {
        _currentArgCount = 0;
        _currentArgs = new RequestArgument[1];
        return;
    }
    _currentArgCount = 1;

    for (int i = 0; i < (int)data.length();)
    {
        i = data.indexOf('&', i);
        if (i == -1)
            break;
        ++i;
        ++_currentArgCount;
    }

    _currentArgs = new RequestArgument[_currentArgCount + 1];

    int pos = 0;
    int iarg;

    for (iarg = 0; iarg < _currentArgCount;)
    {
        int equal_sign_index = data.indexOf('=', pos);
        int next_arg_index = data.indexOf('&', pos);

        if ((equal_sign_index == -1) || ((equal_sign_index > next_arg_index) && (next_arg_index != -1)))
        {
            if (next_arg_index == -1)
                break;
            pos = next_arg_index + 1;
            continue;
        }

        RequestArgument& arg = _currentArgs[iarg];
        arg.key = data.substring(pos, equal_sign_index);
        arg.value = data.substring(equal_sign_index + 1, next_arg_index);

        ++iarg;
        if (next_arg_index == -1)
            break;
        pos = next_arg_index + 1;
    }

    _currentArgCount = iarg;
}

void SERVER::_uploadWriteByte(uint8_t b)
{
    if (_currentUpload.currentSize == HTTP_UPLOAD_BUFLEN)
    {
        if (_currentHandler && _currentHandler->_httpUploadable(_currentUri))
            _currentHandler->_httpUpload(*this, _currentUri, _currentUpload);
        _currentUpload.totalSize += _currentUpload.currentSize;
        _currentUpload.currentSize = 0;
    }
    _currentUpload.buf[_currentUpload.currentSize++] = b;
}

uint8_t SERVER::_uploadReadByte(WiFiClient& client)
{
    int res = client.read();
    if (res == -1)
    {
        while (!client.available() && client.connected())
            yield();
        res = client.read();
    }
    return (uint8_t)res;
}

bool SERVER::_parseForm(WiFiClient& client, String boundary, uint32_t len)
{
    (void)len;

    String line;
    int retry = 0;
    do
    {
        line = client.readStringUntil('\r');
        ++retry;
    } while (line.length() == 0 && retry < 3);

    client.readStringUntil('\n');

    // Start reading the form
    if (line == ("--" + boundary))
    {
        RequestArgument* postArgs = new RequestArgument[32];
        int postArgsLen = 0;
        while (1)
        {
            String argName;
            String argValue;
            String argType;
            String argFilename;
            bool argIsFile = false;

            line = client.readStringUntil('\r');
            client.readStringUntil('\n');
            if (line.length() > 19 && line.substring(0, 19).equalsIgnoreCase("Content-Disposition"))
            {
                int nameStart = line.indexOf('=');
                if (nameStart != -1)
                {
                    argName = line.substring(nameStart + 2);
                    nameStart = argName.indexOf('=');
                    if (nameStart == -1)
                    {
                        argName = argName.substring(0, argName.length() - 1);
                    }
                    else
                    {
                        argFilename = argName.substring(nameStart + 2, argName.length() - 1);
                        argName = argName.substring(0, argName.indexOf('"'));
                        argIsFile = true;

                        // Use GET to set the filename if uploading using blob
                        if (argFilename == "blob" && hasArg("filename"))
                            argFilename = arg("filename");
                    }

                    argType = MIME_TYPE_TEXT;
                    line = client.readStringUntil('\r');
                    client.readStringUntil('\n');

                    if (line.length() > 12 && line.substring(0, 12).equalsIgnoreCase(CONTENT_TYPE_HEADER))
                    {
                        argType = line.substring(line.indexOf(':') + 2);

                        // Skip next line
                        client.readStringUntil('\r');
                        client.readStringUntil('\n');
                    }

                    if (!argIsFile)
                    {
                        while (1)
                        {
                            line = client.readStringUntil('\r');
                            client.readStringUntil('\n');
                            if (line.startsWith("--" + boundary))
                                break;
                            if (argValue.length() > 0)
                                argValue += "\n";
                            argValue += line;
                        }

                        RequestArgument& arg = postArgs[postArgsLen++];
                        arg.key = argName;
                        arg.value = argValue;

                        if (line == ("--" + boundary + "--"))
                        {
                            break;
                        }
                    }
                    else
                    {
                        _currentUpload.status = UPLOAD_FILE_START;
                        _currentUpload.name = argName;
                        _currentUpload.filename = argFilename;
                        _currentUpload.type = argType;
                        _currentUpload.totalSize = 0;
                        _currentUpload.currentSize = 0;

                        if (_currentHandler && _currentHandler->_httpUploadable(_currentUri))
                            _currentHandler->_httpUpload(*this, _currentUri, _currentUpload);

                        _currentUpload.status = UPLOAD_FILE_WRITE;
                        uint8_t argByte = _uploadReadByte(client);

                    readfile:
                        while (argByte != 0x0D)
                        {
                            if (!client.connected())
                                return _parseFormUploadAborted();
                            _uploadWriteByte(argByte);
                            argByte = _uploadReadByte(client);
                        }

                        argByte = _uploadReadByte(client);

                        if (!client.connected())
                            return _parseFormUploadAborted();

                        if (argByte == 0x0A)
                        {
                            argByte = _uploadReadByte(client);

                            if (!client.connected())
                                return _parseFormUploadAborted();

                            if ((char)argByte != '-')
                            {
                                // continue reading the file
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                goto readfile;
                            }
                            else
                            {
                                argByte = _uploadReadByte(client);
                                if (!client.connected())
                                    return _parseFormUploadAborted();
                                if ((char)argByte != '-')
                                {
                                    // continue reading the file
                                    _uploadWriteByte(0x0D);
                                    _uploadWriteByte(0x0A);
                                    _uploadWriteByte((uint8_t)('-'));
                                    goto readfile;
                                }
                            }

                            uint8_t endBuf[boundary.length()];
                            client.readBytes(endBuf, boundary.length());

                            if (strstr((const char*)endBuf, boundary.c_str()) != NULL)
                            {
                                if (_currentHandler && _currentHandler->_httpUploadable(_currentUri))
                                    _currentHandler->_httpUpload(*this, _currentUri, _currentUpload);

                                _currentUpload.totalSize += _currentUpload.currentSize;
                                _currentUpload.status = UPLOAD_FILE_END;

                                if (_currentHandler && _currentHandler->_httpUploadable(_currentUri))
                                    _currentHandler->_httpUpload(*this, _currentUri, _currentUpload);

                                line = client.readStringUntil(0x0D);
                                client.readStringUntil(0x0A);

                                if (line == "--")
                                {
                                    break;
                                }
                                continue;
                            }
                            else
                            {
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                _uploadWriteByte((uint8_t)('-'));
                                _uploadWriteByte((uint8_t)('-'));
                                uint32_t i = 0;
                                while (i < boundary.length())
                                {
                                    _uploadWriteByte(endBuf[i++]);
                                }
                                argByte = _uploadReadByte(client);
                                goto readfile;
                            }
                        }
                        else
                        {
                            _uploadWriteByte(0x0D);
                            goto readfile;
                        }
                        break;
                    }
                }
            }
        }

        int iarg;
        int totalArgs = ((32 - postArgsLen) < _currentArgCount) ? (32 - postArgsLen) : _currentArgCount;

        for (iarg = 0; iarg < totalArgs; iarg++)
        {
            RequestArgument& arg = postArgs[postArgsLen++];
            arg.key = _currentArgs[iarg].key;
            arg.value = _currentArgs[iarg].value;
        }

        if (_currentArgs)
            delete[] _currentArgs;
        _currentArgs = new RequestArgument[postArgsLen];

        for (iarg = 0; iarg < postArgsLen; iarg++)
        {
            RequestArgument& arg = _currentArgs[iarg];
            arg.key = postArgs[iarg].key;
            arg.value = postArgs[iarg].value;
        }

        _currentArgCount = iarg;
        if (postArgs)
            delete[] postArgs;

        return true;
    }

    return false;
}

String SERVER::urlDecode(const String& text)
{
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = text.length();
    unsigned int i = 0;
    while (i < len)
    {
        char decodedChar;
        char encodedChar = text.charAt(i++);
        if ((encodedChar == '%') && (i + 1 < len))
        {
            temp[2] = text.charAt(i++);
            temp[3] = text.charAt(i++);

            decodedChar = strtol(temp, NULL, 16);
        }
        else
        {
            if (encodedChar == '+')
            {
                decodedChar = ' ';
            }
            else
            {
                decodedChar = encodedChar; // normal ascii char
            }
        }
        decoded += decodedChar;
    }
    return decoded;
}

bool SERVER::_parseFormUploadAborted()
{
    _currentUpload.status = UPLOAD_FILE_ABORTED;
    if (_currentHandler && _currentHandler->_httpUploadable(_currentUri))
        _currentHandler->_httpUpload(*this, _currentUri, _currentUpload);
    return false;
}
