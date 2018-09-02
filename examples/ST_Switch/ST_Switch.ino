/*
** EZIoT - Example Smartthings Switch Device
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
#include "ez.h"
#include "st/Switch.h"

/*
** Configuration Defintions
*/
#define EZIOT_WIFI_SSID "{yourSSID}"
#define EZIOT_WIFI_PASS "{yourPASS}"
#define EZIOT_TIME_ZONE "UTC"
#define EZIOT_HTTP_PORT 80

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

/*
** Define our UPnP device
*/
EZ::ST::SWITCH stSwitch1(LED_BUILTIN);

/*
** Program Setup
*/
void setup()
{
    // Add device - this MUST be done before calling iot.start()!
    //
    EZ::iot.root.addDevice('STS1', stSwitch1);

    // Local Configuration
    //
    EZ::iot.timeZone(EZIOT_TIME_ZONE);
    EZ::iot.wifiCredentials(EZIOT_WIFI_SSID, EZIOT_WIFI_PASS);

    // Start the 'thing' running
    //
    EZ::iot.start(EZIOT_HTTP_PORT);
}

/*
** Program Loop
*/
void loop()
{
    // Call the IOT devices loop() method repeatdly
    // Limit what is added to this function as delays
    // will cause issues with the timing of internal
    // functions!
    EZ::iot.loop();
}
