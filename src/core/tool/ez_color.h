/*
** EZIoT - Color Utility Class
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** HSI - work by Brian Neltner (http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white)
** HSB - work by Julio Terra (https://github.com/julioterra/HSB_Color)
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZ_COLOR_H
#define _EZ_COLOR_H
#include <Arduino.h>
#include <math.h>

/*
** Defintions and Equates
*/
#ifndef DEG2RAD
#define DEG2RAD(X) (M_PI * (X) / 180)
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define RGBMIN(r, g, b) (MIN(r, MIN(g, b)))
#define RGBMAX(r, g, b) (MAX(r, MAX(g, b)))

#define EZ_COLOR_SATURATION 255.0

/*
** Color Temperatures (These temperatures are merely characteristic!)
*/
#define EZ_CT_MATCH_FLAME 1700
#define EZ_CT_CANDLE_FLAME 1900
#define EZ_CT_HI_PRESSURE 2200
#define EZ_CT_INCANDESCENT 2800
#define EZ_CT_HALOGEN 3000
#define EZ_CT_WARM_WHITE 2950 // 2700 - 3200
#define EZ_CT_METAL_HALIDE 4000
#define EZ_CT_MOONLIGHT 4125     // 4100 - 4150
#define EZ_CT_NATURAL_WHITE 4250 // 4000 - 4500
#define EZ_CT_SUNLIGHT 4800
#define EZ_CT_HORIZON 5000
#define EZ_CT_DAY_WHITE 5750 // 5500 - 6000
#define EZ_CT_CLOUDY_SKY 6000
#define EZ_CT_COOL_WHITE 7250 // 7000 - 7500
#define EZ_CT_BLUE_SKY 10000  // 10000+

/*
** Some stock color Defintions
*/
#define EZ_COLOR_WHITE EZ::COLOR(255, 255, 255)
#define EZ_COLOR_SILVER EZ::COLOR(192, 192, 192)
#define EZ_COLOR_GRAY EZ::COLOR(128, 128, 128)
#define EZ_COLOR_BLACK EZ::COLOR(0, 0, 0)
#define EZ_COLOR_RED EZ::COLOR(255, 0, 0)
#define EZ_COLOR_MAROON EZ::COLOR(128, 0, 0)
#define EZ_COLOR_YELLOW EZ::COLOR(255, 255, 0)
#define EZ_COLOR_OLIVE EZ::COLOR(128, 128, 0)
#define EZ_COLOR_LIME EZ::COLOR(0, 255, 0)
#define EZ_COLOR_GREEN EZ::COLOR(0, 128, 0)
#define EZ_COLOR_AQUA EZ::COLOR(0, 255, 255)
#define EZ_COLOR_TEAL EZ::COLOR(0, 128, 128)
#define EZ_COLOR_BLUE EZ::COLOR(0, 0, 255)
#define EZ_COLOR_NAVY EZ::COLOR(0, 0, 128)
#define EZ_COLOR_FUCHSIA EZ::COLOR(255, 0, 255)
#define EZ_COLOR_PURPLE EZ::COLOR(128, 0, 128)
#define EZ_COLOR_ORANGE EZ::COLOR(255, 48, 0)

namespace EZ
{
    typedef union COLOR {
        uint32_t u;
        uint8_t q[4];
        struct __attribute__((packed))
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t w;
        };

        // Constructors
        //
        COLOR() : COLOR(0x0L) {}
        COLOR(uint32_t uc) { u = uc; }
        COLOR(uint8_t rc, uint8_t gc, uint8_t bc) : COLOR(rc, gc, bc, 0) {}
        COLOR(uint8_t rc, uint8_t gc, uint8_t bc, uint8_t wc)
        {
            r = rc;
            g = gc;
            b = bc;
            w = wc;
        }

        // Operators
        //
        bool operator==(const uint32_t c) const { return (c == u); }
        bool operator==(const COLOR& c) const { return (c.u == u); }

        COLOR& operator=(const COLOR& c)
        {
            u = c.u;
            return (*this);
        }

        COLOR& operator=(const COLOR* c)
        {
            u = c->u;
            return (*this);
        }

        // Setters
        //
        inline void set(uint8_t rc, uint8_t gc, uint8_t bc) { set(rc, gc, bc, 0); }
        inline void set(uint8_t rc, uint8_t gc, uint8_t bc, uint8_t wc)
        {
            r = rc;
            g = gc;
            b = bc;
            w = wc;
        }

        // Color Wheel
        //
        // Put a value 0 to 255 in to get a color value.
        // The colours are a transition r -> g -> b -> back to r
        //
        COLOR& fromWheel(uint8_t pos)
        {
            w = 0;
            pos = 255 - pos;

            if (pos < 85)
            {
                u = ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
            }
            else if (pos < 170)
            {
                pos -= 85;
                u = ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
            }
            else
            {
                pos -= 170;
                u = ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
            }

            return (*this);
        }

        // Convert from HSI colorspace to RGB
        //
        COLOR& fromHSIf(float H, float S, float I)
        {
            H = fmod(H, 360);                // cycle H around to 0-360 degrees
            H = 3.14159 * H / (float)180;    // Convert to radians.
            S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
            I = I > 0 ? (I < 1 ? I : 1) : 0;

            // Math! Thanks in part to Kyle Miller.
            if (H < 2.09439)
            {
                r = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
                g = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
                b = 255 * I / 3 * (1 - S);
            }
            else if (H < 4.188787)
            {
                H = H - 2.09439;
                g = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
                b = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
                r = 255 * I / 3 * (1 - S);
            }
            else
            {
                H = H - 4.188787;
                b = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
                r = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
                g = 255 * I / 3 * (1 - S);
            }

            w = 0;
            return (*this);
        }

        // Convert from HSI colorspace to RGBW
        //
        // Assumes fully saturated colors, and then scales with white to lower saturation.
        //
        // Next, scale appropriately the pure color by mixing with the white channel.
        // Saturation is defined as "the ratio of colorfulness to brightness" so we will
        // do this by a simple ratio wherein the color values are scaled down by (1-S)
        // while the white LED is placed at S.
        //
        // This will maintain constant brightness because in HSI, R+B+G = I. Thus,
        // S*(R+B+G) = S*I. If we add to this (1-S)*I, where I is the total intensity,
        // the sum intensity stays constant while the ratio of colorfulness to brightness
        // goes down by S linearly relative to total Intensity, which is constant.
        //
        COLOR& fromHSIf2RGBW(float H, float S, float I)
        {
            float cos_h, cos_1047_h;
            H = fmod(H, 360);                // cycle H around to 0-360 degrees
            H = 3.14159 * H / (float)180;    // Convert to radians.
            S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
            I = I > 0 ? (I < 1 ? I : 1) : 0;

            if (H < 2.09439)
            {
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                b = 0;
                w = 255 * (1 - S) * I;
            }
            else if (H < 4.188787)
            {
                H = H - 2.09439;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                r = 0;
                w = 255 * (1 - S) * I;
            }
            else
            {
                H = H - 4.188787;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                b = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                r = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                g = 0;
                w = 255 * (1 - S) * I;
            }

            return (*this);
        }

        // Convert from HSB colorspace to RGB(W)
        //
        COLOR& fromHSBf(float hue, float sat, float bright)
        {
            if (hue > 1)
                hue = 1.0;
            if (sat > 1)
                sat = 1.0;
            if (bright > 1)
                bright = 1.0;
            return fromHSBi(hue * 360.0, sat * 100.0, bright * 100.0);
        }

        COLOR& fromHSBi(int hue, int sat, int bright)
        {
            // constrain all input variables to expected range
            hue = constrain(hue, 0, 360);
            sat = constrain(sat, 0, 100);
            bright = constrain(bright, 0, 100);

            // define maximum value for RGB array elements
            float max_rgb_val = EZ_COLOR_SATURATION;

            // convert saturation and brightness value to decimals and init r, g, b variables
            float sat_f = float(sat) / 100.0;
            float bright_f = float(bright) / 100.0;

            // If brightness is 0 then color is black (achromatic)
            // therefore, R, G and B values will all equal to 0
            if (bright <= 0)
            {
                r = g = b = w = 0;
            }

            // If saturation is 0 then color is gray (achromatic)
            // therefore, R, G and B values will all equal the current brightness
            if (sat <= 0)
            {
                r = g = b = w = bright_f * max_rgb_val;
            }

            // if saturation and brightness are greater than 0 then calculate
            // R, G and B values based on the current hue and brightness
            else
            {
                w = 0;

                if (hue >= 0 && hue < 120)
                {
                    float hue_primary = 1.0 - (float(hue) / 120.0);
                    float hue_secondary = float(hue) / 120.0;
                    float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
                    float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
                    float sat_tertiary = 1.0 - sat_f;
                    r = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
                    g = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
                    b = (bright_f * max_rgb_val) * sat_tertiary;
                }

                else if (hue >= 120 && hue < 240)
                {
                    float hue_primary = 1.0 - ((float(hue) - 120.0) / 120.0);
                    float hue_secondary = (float(hue) - 120.0) / 120.0;
                    float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
                    float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
                    float sat_tertiary = 1.0 - sat_f;
                    r = (bright_f * max_rgb_val) * sat_tertiary;
                    g = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
                    b = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
                }

                else if (hue >= 240 && hue <= 360)
                {
                    float hue_primary = 1.0 - ((float(hue) - 240.0) / 120.0);
                    float hue_secondary = (float(hue) - 240.0) / 120.0;
                    float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
                    float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
                    float sat_tertiary = 1.0 - sat_f;
                    r = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
                    g = (bright_f * max_rgb_val) * sat_tertiary;
                    b = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
                }
            }

            return (*this);
        }

        // Convert from XYB colorspace
        //
        COLOR& fromXYB(float x, float y, int bright)
        {
            float Y = bright / 250.0f;

            float z = 1.0f - x - y;
            float X = (Y / y) * x;
            float Z = (Y / y) * z;

            // sRGB D65 conversion
            // See https://en.wikipedia.org/wiki/SRGB
            float fr = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
            float fg = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
            float fb = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

            if (fr > fb && fr > fg && fr > 1.0f)
            {
                // red is too big
                fg = fg / fr;
                fb = fb / fr;
                fr = 1.0f;
            }
            else if (fg > fb && fg > fr && fg > 1.0f)
            {
                // green is too big
                fr = fr / fg;
                fb = fb / fg;
                fg = 1.0f;
            }
            else if (fb > fr && fb > fg && fb > 1.0f)
            {
                // blue is too big
                fr = fr / fb;
                fg = fg / fb;
                fb = 1.0f;
            }

            // Apply gamma correction
            fr = fr <= 0.0031308f ? 12.92f * fr : (1.0f + 0.055f) * pow(fr, (1.0f / 2.4f)) - 0.055f;
            fg = fg <= 0.0031308f ? 12.92f * fg : (1.0f + 0.055f) * pow(fg, (1.0f / 2.4f)) - 0.055f;
            fb = fb <= 0.0031308f ? 12.92f * fb : (1.0f + 0.055f) * pow(fb, (1.0f / 2.4f)) - 0.055f;

            if (fr > fb && fr > fg)
            {
                // red is biggest
                if (fr > 1.0f)
                {
                    fg = fg / fr;
                    fb = fb / fr;
                    fr = 1.0f;
                }
            }
            else if (fg > fb && fg > fr)
            {
                // green is biggest
                if (fg > 1.0f)
                {
                    fr = fr / fg;
                    fb = fb / fg;
                    fg = 1.0f;
                }
            }
            else if (fb > fr && fb > fg)
            {
                // blue is biggest
                if (fb > 1.0f)
                {
                    fr = fr / fb;
                    fg = fg / fb;
                    fb = 1.0f;
                }
            }

            fr = fr < 0 ? 0 : fr;
            fg = fg < 0 ? 0 : fg;
            fb = fb < 0 ? 0 : fb;

            r = fr * EZ_COLOR_SATURATION;
            g = fg * EZ_COLOR_SATURATION;
            b = fb * EZ_COLOR_SATURATION;
            w = 0;

            return (*this);
        }

        // Convert from Color Temperature
        //
        COLOR& fromTemperature(long kelvin)
        {
            long temperature = kelvin / 100;

            w = 0;

            if (temperature <= 66)
            {
                r = 255;
            }
            else
            {
                r = temperature - 60;
                r = 329.698727446 * pow(r, -0.1332047592);
                if (r < 0)
                    r = 0;
                if (r > 255)
                    r = 255;
            }

            if (temperature <= 66)
            {
                g = temperature;
                g = 99.4708025861 * log(g) - 161.1195681661;
                if (g < 0)
                    g = 0;
                if (g > 255)
                    g = 255;
            }
            else
            {
                g = temperature - 60;
                g = 288.1221695283 * pow(g, -0.0755148492);
                if (g < 0)
                    g = 0;
                if (g > 255)
                    g = 255;
            }

            if (temperature >= 66)
            {
                b = 255;
            }
            else
            {
                if (temperature <= 19)
                {
                    b = 0;
                }
                else
                {
                    b = temperature - 10;
                    b = 138.5177312231 * log(b) - 305.0447927307;
                    if (b < 0)
                        b = 0;
                    if (b > 255)
                        b = 255;
                }
            }
            
            return (*this);
        }

        // Convert to Greyscale
        //
        typedef enum class GREY_MODE
        {
            AVERAGE,
            DESATURATE,
            LUMINANCE,
            BT601,
            BT709,
            DECOMP_MIN,
            DECOMP_MAX
        } grey_render_t;

        // Convert RGB(W) Color to Greyscale
        //
        void makeGrey(grey_render_t mode = GREY_MODE::LUMINANCE) { r = g = b = w = getGrey(mode); }

        uint8_t getGrey(grey_render_t mode = GREY_MODE::LUMINANCE)
        {
            uint8_t grey = w;

            if ((r + g + b) != 0)
            {
                switch (mode)
                {
                    case GREY_MODE::BT601:
                        grey = (r * 0.299 + g * 0.587 + b * 0.114);
                        break;
                    case GREY_MODE::BT709:
                        grey = (r * 0.2126 + g * 0.7152 + b * 0.0722);
                        break;
                    case GREY_MODE::LUMINANCE:
                        grey = (r * 0.3 + g * 0.59 + b * 0.11);
                        break;
                    case GREY_MODE::DESATURATE:
                        grey = (RGBMAX(r, g, b) + RGBMIN(r, g, b)) / 2;
                        break;
                    case GREY_MODE::DECOMP_MIN:
                        grey = RGBMIN(r, g, b);
                        break;
                    case GREY_MODE::DECOMP_MAX:
                        grey = RGBMAX(r, g, b);
                        break;
                    case GREY_MODE::AVERAGE:
                        grey = (r + g + b) / 3;
                    default:
                        break;
                }
            }

            return grey;
        }

    } color_t;

} // namespace EZ
#endif // _EZ_COLOR_H
