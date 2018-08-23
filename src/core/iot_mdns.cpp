/*
** EZIoT - IOT Controller: mDNS
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
#include <mdns.h>

using namespace EZ;

static void _mdnsEvent(system_event_t* event) { mdns_handle_system_event(NULL, event); }

void IOT::mdnsInstance(String name)
{
    if (name.length() > 63)
        return;

    if (xEventGroupGetBits(_eventGroup) & MDNS_BIT)
    {
        if (mdns_instance_name_set(name.c_str()) != ESP_OK)
            console.printf(LOG::WARNING, "mDNS: failed setting instance name, %s.", name.c_str());
    }
}

void IOT::mdnsService(const char* name, const char* proto, uint16_t port, const char* instName, mdns_txt_item_t* txt,
                      int len)
{
    if (xEventGroupGetBits(_eventGroup) & MDNS_BIT)
    {
        console.printf(LOG::INFO2, "mDNS: Service, %s.%s.", name, proto);

        if (mdns_service_add(instName, name, proto, port, txt, len) != ESP_OK)
            console.printf(LOG::WARNING, "mDNS: failed to add service, %s.%s.", name, proto);

        if (instName)
        {
            if (mdns_service_instance_name_set(name, proto, instName) != ESP_OK)
                console.printf(LOG::WARNING, "mDNS: failed setting service instance name, %s", instName);
        }
    }
}

void IOT::mdnsServiceTxt(const char* name, const char* proto, const char* key, const char* value)
{
    if (xEventGroupGetBits(_eventGroup) & MDNS_BIT)
    {
        if (mdns_service_txt_item_set(name, proto, key, value) != ESP_OK)
            console.printf(LOG::WARNING, "mDNS: failed setting service TXT, %s.%s - %s.%s.", name, proto, key, value);
    }
}

void IOT::mdnsRemove(const char* name, const char* proto)
{
    if (xEventGroupGetBits(_eventGroup) & MDNS_BIT)
    {
        if (mdns_service_remove(name, proto) != ESP_OK)
            console.printf(LOG::WARNING, "mDNS: failed removing service, %s.%s.", name, proto);
    }
}

void IOT::_mdnsStart(void)
{
    _mdnsStop();

    if (mdns_init() == ESP_OK)
    {
        WiFi.onEvent(_mdnsEvent);
        xEventGroupSetBits(_eventGroup, MDNS_BIT);

        if (mdns_hostname_set(WiFi.getHostname()) == ESP_OK)
        {
            console.printf(LOG::INFO1, "mDNS: Started.");
            // mdnsInstance(iotTag);
        }
        else
            console.printf(LOG::WARNING, "mDNS: failed to set hostname");
    }
    else
        console.printf(LOG::ERROR, "mDNS: failed to start");
}

void IOT::_mdnsStop(void)
{
    if (xEventGroupGetBits(_eventGroup) & MDNS_BIT)
    {
        xEventGroupClearBits(_eventGroup, MDNS_BIT);
        mdns_free();
        console.printf(LOG::INFO1, "mDNS: Stopped.");
    }
}
