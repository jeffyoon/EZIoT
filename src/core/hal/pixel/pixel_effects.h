/*
** EZIoT - Pixel Effect Strings
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** Based upon work by Adafruit (NeoPixel Libray)
**
** Pixel Animation/FX based upon work by Harm Aldick/Keith Lord (WS2812FX Library)
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZ_PIXEL_EFFECTS_H
#define _EZ_PIXEL_EFFECTS_H
#pragma once

#define FSH(x) (__FlashStringHelper*)(x)

namespace EZ
{
    namespace PIXEL
    {
        // Custom FX class
        //
        class HANDLER;

        class EFFECT {
            public:
                EFFECT();
                virtual uint16_t call(HANDLER& pixels) = 0;
                const char *name(void) const { return _name; }
            private:
                const char *_name;
        };

        // Built-in FX names 
        //
        const char fx_0[] PROGMEM = "Off";
        const char fx_1[] PROGMEM = "On (Static)";
        const char fx_2[] PROGMEM = "Breathing";
        const char fx_3[] PROGMEM = "Blink";
        const char fx_4[] PROGMEM = "Blink Duo";
        const char fx_5[] PROGMEM = "Blink Rainbow";
        const char fx_6[] PROGMEM = "Strobe";
        const char fx_7[] PROGMEM = "Strobe Duo";
        const char fx_8[] PROGMEM = "Strobe Rainbow";
        const char fx_9[] PROGMEM = "Multi Strobe";
        const char fx_10[] PROGMEM = "Color Wipe";
        const char fx_11[] PROGMEM = "Color Wipe Inverse";
        const char fx_12[] PROGMEM = "Color Wipe Reverse";
        const char fx_13[] PROGMEM = "Color Wipe Reverse Inverse";
        const char fx_14[] PROGMEM = "Color Wipe Random";
        const char fx_15[] PROGMEM = "Color Sweep Random";
        const char fx_16[] PROGMEM = "Random Color";
        const char fx_17[] PROGMEM = "Single Dynamic";
        const char fx_18[] PROGMEM = "Multi Dynamic";
        const char fx_19[] PROGMEM = "Rainbow";
        const char fx_20[] PROGMEM = "Rainbow Cycle";
        const char fx_21[] PROGMEM = "Fader";
        const char fx_22[] PROGMEM = "Theater Chase";
        const char fx_23[] PROGMEM = "Theater Chase Rainbow";
        const char fx_24[] PROGMEM = "Twinkle";
        const char fx_25[] PROGMEM = "Twinkle Random";
        const char fx_26[] PROGMEM = "Twinkle Fade";
        const char fx_27[] PROGMEM = "Twinkle Fade Random";
        const char fx_28[] PROGMEM = "Sparkle";
        const char fx_29[] PROGMEM = "Flash Sparkle";
        const char fx_30[] PROGMEM = "Hyper Sparkle";
        const char fx_31[] PROGMEM = "Running Lights";
        const char fx_32[] PROGMEM = "Running Random";
        const char fx_33[] PROGMEM = "Running Color";
        const char fx_34[] PROGMEM = "Running - Emergency";
        const char fx_35[] PROGMEM = "Running - Halloween";
        const char fx_36[] PROGMEM = "Running - Christmas";
        const char fx_37[] PROGMEM = "Scan";
        const char fx_38[] PROGMEM = "Dual Scan";
        const char fx_39[] PROGMEM = "Larson Scanner";
        const char fx_40[] PROGMEM = "Larson Rainbow";
        const char fx_41[] PROGMEM = "ICU";
        const char fx_42[] PROGMEM = "Chase White";
        const char fx_43[] PROGMEM = "Chase Color";
        const char fx_44[] PROGMEM = "Chase Random";
        const char fx_45[] PROGMEM = "Chase Rainbow";
        const char fx_46[] PROGMEM = "Chase Rainbow White";
        const char fx_47[] PROGMEM = "Chase Blackout";
        const char fx_48[] PROGMEM = "Chase Blackout Rainbow";
        const char fx_49[] PROGMEM = "Chase Flash";
        const char fx_50[] PROGMEM = "Chase Flash Random";
        const char fx_51[] PROGMEM = "Bicolor Chase";
        const char fx_52[] PROGMEM = "Tricolor Chase";
        const char fx_53[] PROGMEM = "Circus Combustus";
        const char fx_54[] PROGMEM = "Comet";
        const char fx_55[] PROGMEM = "Fireworks";
        const char fx_56[] PROGMEM = "Fireworks Random";
        const char fx_57[] PROGMEM = "Fire Flicker";
        const char fx_58[] PROGMEM = "Fire Flicker (soft)";
        const char fx_59[] PROGMEM = "Fire Flicker (intense)";
        const char fx_60[] PROGMEM = "Custom #1";
        const char fx_61[] PROGMEM = "Custom #2";
        const char fx_62[] PROGMEM = "Custom #3";

        static const __FlashStringHelper* modeNames[] = {
            FSH(fx_0),  FSH(fx_1),  FSH(fx_2),  FSH(fx_3),  FSH(fx_4),  FSH(fx_5),  FSH(fx_6),  FSH(fx_7),  FSH(fx_8),
            FSH(fx_9),  FSH(fx_10), FSH(fx_11), FSH(fx_12), FSH(fx_13), FSH(fx_14), FSH(fx_15), FSH(fx_16), FSH(fx_17),
            FSH(fx_18), FSH(fx_19), FSH(fx_20), FSH(fx_21), FSH(fx_22), FSH(fx_23), FSH(fx_24), FSH(fx_25), FSH(fx_26),
            FSH(fx_27), FSH(fx_28), FSH(fx_29), FSH(fx_30), FSH(fx_31), FSH(fx_32), FSH(fx_33), FSH(fx_34), FSH(fx_35),
            FSH(fx_36), FSH(fx_37), FSH(fx_38), FSH(fx_39), FSH(fx_40), FSH(fx_41), FSH(fx_42), FSH(fx_43), FSH(fx_44),
            FSH(fx_45), FSH(fx_46), FSH(fx_47), FSH(fx_48), FSH(fx_49), FSH(fx_50), FSH(fx_51), FSH(fx_52), FSH(fx_53),
            FSH(fx_54), FSH(fx_55), FSH(fx_56), FSH(fx_57), FSH(fx_58), FSH(fx_59), FSH(fx_60), FSH(fx_61), FSH(fx_62)};

        static const uint8_t MODE_COUNT = (sizeof(modeNames) / sizeof(modeNames[0]));
        static const uint8_t CUSTOM_FX3 = (MODE_COUNT - 1);
        static const uint8_t CUSTOM_FX2 = (CUSTOM_FX3 - 1);
        static const uint8_t CUSTOM_FX1 = (CUSTOM_FX2 - 1);

    } // namespace PIXEL
} // namespace EZ
#endif // _EZ_PIXEL_EFFECTS_H