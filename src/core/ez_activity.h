/*
** EZIoT - Activity Base Class
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
#if !defined(_EZ_ACTIVITY_H)
#define _EZ_ACTIVITY_H
#include "ez_common.h"
#include "ez_service.h"

namespace EZ
{
    class ACTIVITY
    {
        friend class IOT;
        friend class SERVICE;

    public:
        enum class MODE
        {
            VARIABLE,
            ACTION,
        };

        virtual ~ACTIVITY() {}
        ACTIVITY(const char* name, MODE mode);
        MODE mode(void) { return _mode; }
        String name(void) { return _name; }
        SERVICE* baseService(void) const { return _baseService; }
        ACTIVITY* nextActivity(void) const { return _nextActivity; }
        ACTIVITY* prevActivity(void) const { return _prevActivity; }
        virtual String upnpXML(bool valueTag = false, bool emptyTag = false) = 0;

    protected:
        int _takeServiceMutex(TickType_t xTicks = portMAX_DELAY);
        int _giveServiceMutex(void);
        bool _postCallback(SERVICE::CALLBACK type, void *vp = nullptr);

    private:
        MODE _mode;
        String _name;
        SERVICE* _baseService;
        ACTIVITY* _prevActivity;
        ACTIVITY* _nextActivity;

        ACTIVITY(ACTIVITY const& copy);            // Not Implemented
        ACTIVITY& operator=(ACTIVITY const& copy); // Not Implemented
    };
} // namespace EZ
#endif // _EZ_ACTIVITY_H