/*
** EZIoT - Example UPnP BinaryLight Device
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
#include "upnp/upnp.h"

/*
** Configuration Defintions
*/
#define EZIOT_WIFI_SSID "{yourSSID}"
#define EZIOT_WIFI_PASS "{yourPASS}"
#define EZIOT_TIME_ZONE "UTC"
#define EZIOT_HTTP_PORT 80

#define BLIGHT_PIN 2

/*
** Forward Reference
*/
bool bLightActivity(EZ::ACTIVITY* activity, EZ::SERVICE::CALLBACK type, void* vp);

/*
** Define our UPnP device
*/
EZ::UPNP::BINARYLIGHT bLight(bLightActivity);

/*
** Program Setup
*/
void setup()
{
    // Add device - this MUST be done before call iot.start()!
    //
    EZ::iot.addDevice('BL01', bLight);

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

/*
** Activity Callback
*/
bool bLightActivity(EZ::ACTIVITY* activity, EZ::SERVICE::CALLBACK type, void* vp)
{
    //
    // On Initialisation, we setup the digital pin and set its state from
    // the state variable in case we are saving last state to NVS
    //
    if (type == EZ::SERVICE::CALLBACK::INIT && vp == &bLight.SwitchPower)
    {
        Serial.printf("Binary Light: Initialise\n");

        // Set pin to OUTPUT
        pinMode(BLIGHT_PIN, OUTPUT);

        // This will match the next case below, so we set the start-up state
        type = EZ::SERVICE::CALLBACK::POST_CHANGE;
        activity = &bLight.SwitchPower.Target;
    }

    //
    // Do we have a 'Target' state change?
    //
    if (type == EZ::SERVICE::CALLBACK::POST_CHANGE && activity == &bLight.SwitchPower.Target)
    {
        // What is the 'Target' state required?
        bool target = bLight.SwitchPower.Target.native();

        // Set the pin to the 'Target' state
        digitalWrite(BLIGHT_PIN, target);
    }

    if (activity == &bLight.SwitchPower.Status && type == EZ::SERVICE::CALLBACK::POST_CHANGE)
    {
        bool target = bLight.SwitchPower.Target.native();
        Serial.printf("Binary Light: Status: %s (%d)\n", bLight.SwitchPower.Status.native() ? "ON" : "OFF", target);
    }

    if (type == EZ::SERVICE::CALLBACK::LOOP)
    {
        // Set the 'Status' variable to the actual pin value
        bLight.SwitchPower.Status.native(digitalRead(BLIGHT_PIN));
    }

    // TRUE = Allow activity to continue as normal
    //
    // Read the documentation before returning a FALSE value
    //
    return true;
}
