/*
** EZIoT - IOT Controller: Root Device
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
#include "iot.h"

using namespace EZ;

/*
** Page Templates
*/
const char HTTP_HEAD[] PROGMEM =
    "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, "
    "user-scalable=no\"/><title>EZIoT - {v}</title>";

const char HTTP_STYLE[] PROGMEM =
    "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: "
    "center;font-family:verdana;} "
    "button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;"
    "width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: "
    "url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///"
    "8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/"
    "uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1H"
    "GfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";

const char HTTP_SCRIPT[] PROGMEM = "<script>function "
                                   "c(l){document.getElementById('s').value=l.innerText||l.textContent;document."
                                   "getElementById('p').focus();}</script>";

const char HTTP_HEAD_END[] PROGMEM = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";

const char HTTP_END[] PROGMEM = "</div></body></html>";

ROOT::ROOT() : DEVICE(), HTTP::SERVER()
{
    // 'rO0T' = 1917792340
    _iotCode = 1917792340;
    _upnpVersionMajor = 2;
    _upnpVersionMinor = 0;
    _upnpDeviceType = "Thing:1";
    _httpServer = this;
    _httpSetup(this);
    httpHandler(this);

    _upnpFriendlyName.value("Thing (" + upnpSerialNumber() + ")");
}

String ROOT::upnpUUID(void)
{
    String uuid(EZ_UPNP_UUID_DEVICE_PREFIX);
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();
    uuid.concat(mac);

    return uuid;
}

uint16_t ROOT::httpPort(void) { return _httpPort; }

/*
** Web Handlers
*/
bool ROOT::_httpAccept(HTTP::METHOD method, String uri)
{
    if (method == HTTP::METHOD::GET && (uri == "/" || uri == "/eziot/index.html"))
        return true;

    return DEVICE::_httpAccept(method, uri);
}

bool ROOT::_httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri)
{
    // ESP_LOGV(eziotString, "%s", uri.c_str());

    if (method == HTTP::METHOD::GET && (uri == "/" || uri == "/eziot/index.html"))
        return _httpPresentation(server, method, uri);
    return DEVICE::_httpHandle(server, method, uri);
}

/*
** Presentation Page
*/
bool ROOT::_httpPresentation(HTTP::SERVER& server, HTTP::METHOD method, String uri)
{
    String page = FPSTR(HTTP_HEAD);

    page.replace("{v}", "Options");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    // page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += String(F("<h1>"));
    // page += _apName;
    page += String(F("</h1>"));
    page += String(F("<h3>EZIoT</h3>"));
    // page += FPSTR(HTTP_PORTAL_OPTIONS);
    page += FPSTR(HTTP_END);

    return server.send(200, MIME_TYPE_HTML, page);
}
