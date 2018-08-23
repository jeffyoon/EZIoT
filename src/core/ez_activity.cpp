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
#include "ez_activity.h"
#include "ez_service.h"

using namespace EZ;

ACTIVITY::ACTIVITY(const char* name, MODE mode)
    : _mode(mode), _name(name), _baseService(nullptr), _prevActivity(nullptr), _nextActivity(nullptr)
{
}

int ACTIVITY::_takeServiceMutex(TickType_t xTicks)
{
    if (_baseService)
        return xSemaphoreTake(_baseService->mutexLock(), xTicks);
    return -1;
}

int ACTIVITY::_giveServiceMutex(void)
{
    if (_baseService)
        return xSemaphoreGive(_baseService->mutexLock());
    return -1;
}

bool ACTIVITY::_postCallback(SERVICE::CALLBACK type, void *vp)
{
    if ((_baseService) && _baseService->_onActivityCb && type != SERVICE::CALLBACK::LOOP)
        return _baseService->_onActivityCb(this, type, vp);
    return true;
}