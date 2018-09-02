/*
** EZIoT - Service Base Class
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
#include "ez_service.h"
#include "ez_activity.h"
#include "ez_device.h"

using namespace EZ;

SERVICE::SERVICE(MODE mode, const char* name)
    : _mode(mode), _name(name), _baseDevice(nullptr), _prevService(nullptr), _nextService(nullptr),
      _headActivity(nullptr), _tailActivity(nullptr), _onActivityCb(nullptr), _iotCode(0), _nvsHandle(0)
{
    // _mutexLock = xSemaphoreCreateMutex();
    _mutexLock = xSemaphoreCreateRecursiveMutex();
}

SERVICE::~SERVICE() { vSemaphoreDelete(_mutexLock); };

ACTIVITY& SERVICE::addActivity(ACTIVITY& newActivity)
{
    (void)addActivity(&newActivity);
    return newActivity;
}

ACTIVITY* SERVICE::addActivity(ACTIVITY* newActivity)
{
    if ((newActivity) && !newActivity->_homeService)
    {
        if (_headActivity == nullptr)
        {
            newActivity->_nextActivity = nullptr;
            newActivity->_prevActivity = nullptr;
            _headActivity = newActivity;
            _tailActivity = newActivity;
        }
        else
        {
            newActivity->_nextActivity = nullptr;
            newActivity->_prevActivity = _tailActivity;
            newActivity->_prevActivity->_nextActivity = newActivity;
            _tailActivity = newActivity;
        }

        newActivity->_homeService = this;

        if (_mode == MODE::CONFIG && newActivity->_mode == ACTIVITY::MODE::VARIABLE)
        {
            VARIABLE* var = reinterpret_cast<VARIABLE*>(newActivity);
            var->_nvs = true;
        }
    }

    return newActivity;
}

String SERVICE::urlBase(const char* path)
{
    uint16_t port = 80; //_web.webPort();
    String url("http://");

    url.concat(WiFi.localIP().toString());
    if (1)
    {
        url.concat(":");
        url.concat(port);
    }
    if ((path) && *path != '\0')
        url.concat(path);
    return url;
}

String SERVICE::uuidDevice(void)
{
    if (_baseDevice)
        return _baseDevice->upnpUUID();
    return "";
}

void SERVICE::_sendCommonHeaders(HTTP::SERVER& server, bool incServer)
{
    if ((incServer) && _baseDevice)
    {
        server.sendHeader("SERVER", _baseDevice->upnpServer().c_str());
    }

    // server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    // server.sendHeader("Pragma", "no-cache");
    // server.sendHeader("Expires", "-1");

    server.sendHeader("DATE", dateRFC1123());
    server.sendHeader("CONTENT-LANGUAGE", "en");
}
