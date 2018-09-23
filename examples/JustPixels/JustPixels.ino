/*
** EZIoT - Example Just Pixel Strand(s)
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
//#include "ez.h"
#include "core/hal/hal_dmx.h"
#include "core/hal/hal_neo.h"

using namespace EZ;

// e.g. TTGO TAudio Board
#define STRAND_PIN GPIO_NUM_22
#define STRAND_LEN 19           // Length in pixels (not in # of LEDS!)

// Set-up strand details
//
NEO::STRAND strand(PIXEL::ORDER::RGB, STRAND_LEN, NEO::TYPE::WS2812B_V1, STRAND_PIN, RMT_CHANNEL_0);

// Assign strand to NEO Pixel DRIVER (maximum of 4 strands are supported)
//
NEO::DRIVER neo(strand);

//PIXEL::SEGMENT seg1(strand, 6, 5);

void setup()
{
    Serial.begin(115200);
    neo.start();
    strand.setMode(0);
}

void loop()
{
    // Call each strand service
    //
    strand.service();

    // Cycle through all the FX Modes
    //
    static unsigned long last_change = 0;
    unsigned long now = millis();

    if (now - last_change > 15000)
    {
        strand.setMode((strand.getMode() + 1) % strand.getModeCount());
        Serial.println(strand.getModeName());
        last_change = now;
    }

    // Render all the strands in one go
    //
    neo.render();
}
