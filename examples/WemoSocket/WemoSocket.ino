/*
** EZIoT - Example Wemo Socket Device
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
#include "wemo/Socket.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

/*
** Configuration Defintions
*/
#define EZIOT_WIFI_SSID "{yourSSID}"
#define EZIOT_WIFI_PASS "{yourPASS}"
#define EZIOT_TIME_ZONE "UTC"
#define EZIOT_HTTP_PORT 80

/*
** Forward Reference
*/
bool wSocketActivity(EZ::ACTIVITY* activity, EZ::SERVICE::CALLBACK type, void* vp);

/*
** Define the UPNP device
*/
EZ::WEMO::SOCKET wSocket(wSocketActivity);

/*
** Program Setup
*/
void setup()
{
    // Add device - this MUST be done before call iot.start()!
    //
    EZ::iot.addDevice(wSocket);

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
bool wSocketActivity(EZ::ACTIVITY* activity, EZ::SERVICE::CALLBACK type, void* vp)
{
    // WEMO Socket
    //
    // On Initialisation, we setup the digital pin and set its state from
    // the state variable in case we are saving last state to NVS
    //
    if (vp == &wSocket.BasicEvent && type == EZ::SERVICE::CALLBACK::INIT)
    {
        Serial.printf("WEMO Socket: Initialise\n");

        // Set pin to OUTPUT
        pinMode(LED_BUILTIN, OUTPUT);

        // This will match the next function so we set start-up state
        activity = &wSocket.BasicEvent.BinaryState;
        type = EZ::SERVICE::CALLBACK::POST_CHANGE;
    }

    if (activity == &wSocket.BasicEvent.BinaryState && type == EZ::SERVICE::CALLBACK::POST_CHANGE)
    {
        bool state = wSocket.BasicEvent.BinaryState.native();

        digitalWrite(LED_BUILTIN, state);
        Serial.printf("WEMO Socket: State: %s (%d)\n", state ? "ON" : "OFF", digitalRead(LED_BUILTIN));
    }

    // TRUE = Allow activity to continue as normal
    //
    // Read the documentation before returning a FALSE value
    //
    return true;
}
