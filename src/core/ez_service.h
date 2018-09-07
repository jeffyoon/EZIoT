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
#if !defined(_EZ_SERVICE_H)
#define _EZ_SERVICE_H
#include "ez_common.h"
#include "ez_http.h"

namespace EZ
{
    /*
    ** Forward references
    */
    class IOT;
    class DEVICE;
    class ACTIVITY;
    class VARIABLE;

    class SERVICE : protected HTTP::HANDLER
    {
        friend class ACTIVITY;
        friend class DEVICE;
        friend class IOT;

    public:
        typedef struct _event_t
        {
            SERVICE* service;
            ACTIVITY* activity;
        } event_t;

        enum class MODE
        {
            CONFIG,
            CUSTOM,
            UPNP
        };

        enum class CALLBACK
        {
            INIT = 1,
            START,
            LOOP,
            STOP,
            PRE_ACTION,
            PRE_CHANGE,
            POST_CHANGE,
            POST_ACTION
        };

        //typedef bool (*onActivityCb)(ACTIVITY* activity, CALLBACK type, void *vp);
        typedef std::function<bool(ACTIVITY* activity, CALLBACK type, void *vp)> onActivityCb;

        virtual ~SERVICE();
        SERVICE(MODE mode, const char* name);

        MODE mode(void) { return _mode; }

        ACTIVITY& addActivity(ACTIVITY& newActivity);
        ACTIVITY* addActivity(ACTIVITY* newActivity);
        ACTIVITY* headActivity(void) const { return _headActivity; }
        ACTIVITY* tailActivity(void) const { return _tailActivity; }
        DEVICE* baseDevice(void) const { return _baseDevice; }
        SERVICE* nextService(void) const { return _nextService; }
        SERVICE* prevService(void) const { return _prevService; }

        SemaphoreHandle_t mutexLock(void) const { return _mutexLock; }
        uint32_t nvsHandle(void) const { return _nvsHandle; }

        String urlBase(const char* path = nullptr);
        String uuidDevice(void);

        virtual void registerEvent(ACTIVITY* activity) {}
        virtual void processEvent(event_t* event) {}
        virtual void onActivity(onActivityCb cb) { _onActivityCb = cb; }

    protected:
        MODE _mode;
        const char* _name;
        DEVICE* _baseDevice;
        SERVICE* _prevService;
        SERVICE* _nextService;
        ACTIVITY* _headActivity;
        ACTIVITY* _tailActivity;
        onActivityCb _onActivityCb;

        virtual void _initialise(void) = 0;
        virtual bool _httpAccept(HTTP::METHOD method, String uri) = 0;
        virtual bool _httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri) = 0;
        void _sendCommonHeaders(HTTP::SERVER& server, bool incServer = true);
        void _eventQueue(event_t* e);

    private:
        uint32_t _iotCode;
        uint32_t _nvsHandle;
        SemaphoreHandle_t _mutexLock;
        SERVICE(SERVICE const& copy);            // Not Implemented
        SERVICE& operator=(SERVICE const& copy); // Not Implemented
    };
} // namespace EZ
#endif // _EZ_SERVICE_H
/******************************************************************************/
