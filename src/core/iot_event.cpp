/*
** EZIoT - IOT Controller: Event Task Queue
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
#include "iot.h"

using namespace EZ;

/*
** Event Task
*/
static volatile TaskHandle_t _event_handle = NULL;
static xQueueHandle _event_queue;

void IOT::_eventTask(void* pv)
{
    SERVICE::event_t* e = NULL;
    EventBits_t bits;

    while ((bits = xEventGroupGetBits(iot._eventGroup)) & iot.EVENT_BIT)
    {
        if ((bits & (iot.CONNECTED_BIT | iot.EZIOT_BIT)) && !(bits & iot.OTAU_BIT))
        {
            if (xQueueReceive(_event_queue, &e, portMAX_DELAY) == pdTRUE)
            {
                if (e)
                {
                    ESP_LOGV(eziotString, "Event Task: Processing.");
                    if (e->service)
                        e->service->processEvent(e);
                    free((void*)(e));
                }
            }
        }
        else
            vTaskDelay(10);
    }

    iot.console.printf(LOG::INFO1, "Events: Task Stopped.");
    _event_handle = NULL;
    vTaskDelete(NULL);
}

void IOT::_eventStart(void)
{
    if (!_event_queue)
    {
        if (!(_event_queue = xQueueCreate(32, sizeof(SERVICE::event_t*))))
        {
            console.printf(LOG::ERROR, "Events: failed to create xQueue.");
            return;
        }
    }

    if (!_event_handle)
    {
        xEventGroupSetBits(_eventGroup, EVENT_BIT);
        xTaskCreate(_eventTask, "iotEvents", 4096, this, 3, (TaskHandle_t*)&_event_handle);

        if (!_event_handle)
        {
            console.printf(LOG::ERROR, "Events: failed to create task.");
            xEventGroupClearBits(_eventGroup, SSDP_BIT);
            return;
        }
    }
}

void IOT::_eventStop(void)
{
    xEventGroupClearBits(_eventGroup, EVENT_BIT);

    if (_event_handle)
    {
        // Hmmm, maybe want to think about setting a timer
        // and killing the task if it expires and not stopped
        // gracefully!
        //
        while (_event_handle)
        {
            vTaskDelay(10);
        }
    }

    if (_event_queue)
    {
        SERVICE::event_t* e;

        while (xQueueReceive(_event_queue, &e, 0) == pdTRUE)
        {
            free((void*)(e));
        }

        vQueueDelete(_event_queue);
        _event_queue = NULL;
    }
}

void SERVICE::_eventQueue(event_t* e)
{
    if (!e)
        return;

    if (_event_queue)
    {
        if (xQueueSend(_event_queue, &e, portMAX_DELAY) == pdPASS)
            return;
    }

    ESP_LOGE(iotTag, "Event Post: Queue failed.");
    free((void*)(e));
}
