/*
** EZIoT - Pixel Handler Class
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
#ifndef _EZ_PIXEL_HANDLER_H
#define _EZ_PIXEL_HANDLER_H
#include "core/tool/ez_color.h"
#include "core/tool/ez_gamma.h"
#include "core/tool/ez_sine.h"
#include "pixel_sequence.h"
#include "pixel_effects.h"

#define EZ_PIXEL_SPEED_MIN (uint16_t)2
#define EZ_PIXEL_SPEED_MAX (uint16_t)65535
#define EZ_PIXEL_LEVEL_MIN (uint8_t)0
#define EZ_PIXEL_LEVEL_MAX (uint8_t)255

#define EZ_PIXEL_DEFAULT_LEVEL (uint8_t)50
#define EZ_PIXEL_DEFAULT_MODE (uint8_t)0
#define EZ_PIXEL_DEFAULT_SPEED (uint16_t)1000

#define EZ_PIXEL_ONOFF_DELAY (uint16_t)50

#define EZ_PIXEL_STATE_OFF 0
#define EZ_PIXEL_STATE_TURNING_OFF 1
#define EZ_PIXEL_STATE_TURNING_ON 2
#define EZ_PIXEL_STATE_ON 3

// Options
//
// bit    8: reverse animation
// bits 5-7: fade rate (0-7)
// bit    4: gamma correction
// bits 1-3: TBD
#define EZ_PIXEL_OPTION_NONE (uint8_t)0x00
#define EZ_PIXEL_OPTION_REVERSE (uint8_t)0x80
#define EZ_PIXEL_IS_REVERSE ((_options & EZ_PIXEL_OPTION_REVERSE) == EZ_PIXEL_OPTION_REVERSE)
#define EZ_PIXEL_OPTION_FADE_XFAST (uint8_t)0x10
#define EZ_PIXEL_OPTION_FADE_FAST (uint8_t)0x20
#define EZ_PIXEL_OPTION_FADE_MEDIUM (uint8_t)0x30
#define EZ_PIXEL_OPTION_FADE_SLOW (uint8_t)0x40
#define EZ_PIXEL_OPTION_FADE_XSLOW (uint8_t)0x50
#define EZ_PIXEL_OPTION_FADE_XXSLOW (uint8_t)0x60
#define EZ_PIXEL_OPTION_FADE_GLACIAL (uint8_t)0x70
#define EZ_PIXEL_OPTION_FADE_RATE ((_options & 0x70) >> 4)
#define EZ_PIXEL_OPTION_GAMMA (uint8_t)0x08
#define EZ_PIXEL_IS_GAMMA ((_options & EZ_PIXEL_OPTION_GAMMA) == EZ_PIXEL_OPTION_GAMMA)

namespace EZ
{
    namespace PIXEL
    {
        // Forward refernce
        //
        class HANDLER;
        class SEGMENT;

        // Pixel LED Order (see the Adafruit NeoPixel Library for more details)
        //
        typedef enum ORDER
        {
            MONO = (0),

            RGB = ((0 << 6) | (0 << 4) | (1 << 2) | (2)),
            RBG = ((0 << 6) | (0 << 4) | (2 << 2) | (1)),
            GRB = ((1 << 6) | (1 << 4) | (0 << 2) | (2)),
            GBR = ((2 << 6) | (2 << 4) | (0 << 2) | (1)),
            BRG = ((1 << 6) | (1 << 4) | (2 << 2) | (0)),
            BGR = ((2 << 6) | (2 << 4) | (1 << 2) | (0)),

            WRGB = ((0 << 6) | (1 << 4) | (2 << 2) | (3)),
            WRBG = ((0 << 6) | (1 << 4) | (3 << 2) | (2)),
            WGRB = ((0 << 6) | (2 << 4) | (1 << 2) | (3)),
            WGBR = ((0 << 6) | (3 << 4) | (1 << 2) | (2)),
            WBRG = ((0 << 6) | (2 << 4) | (3 << 2) | (1)),
            WBGR = ((0 << 6) | (3 << 4) | (2 << 2) | (1)),

            RWGB = ((1 << 6) | (0 << 4) | (2 << 2) | (3)),
            RWBG = ((1 << 6) | (0 << 4) | (3 << 2) | (2)),
            RGWB = ((2 << 6) | (0 << 4) | (1 << 2) | (3)),
            RGBW = ((3 << 6) | (0 << 4) | (1 << 2) | (2)),
            RBWG = ((2 << 6) | (0 << 4) | (3 << 2) | (1)),
            RBGW = ((3 << 6) | (0 << 4) | (2 << 2) | (1)),

            GWRB = ((1 << 6) | (2 << 4) | (0 << 2) | (3)),
            GWBR = ((1 << 6) | (3 << 4) | (0 << 2) | (2)),
            GRWB = ((2 << 6) | (1 << 4) | (0 << 2) | (3)),
            GRBW = ((3 << 6) | (1 << 4) | (0 << 2) | (2)),
            GBWR = ((2 << 6) | (3 << 4) | (0 << 2) | (1)),
            GBRW = ((3 << 6) | (2 << 4) | (0 << 2) | (1)),

            BWRG = ((1 << 6) | (2 << 4) | (3 << 2) | (0)),
            BWGR = ((1 << 6) | (3 << 4) | (2 << 2) | (0)),
            BRWG = ((2 << 6) | (1 << 4) | (3 << 2) | (0)),
            BRGW = ((3 << 6) | (1 << 4) | (2 << 2) | (0)),
            BGWR = ((2 << 6) | (3 << 4) | (1 << 2) | (0)),
            BGRW = ((3 << 6) | (2 << 4) | (1 << 2) | (0))
        } pixel_order_t;

        static const uint16_t MAX_SFX_COLORS = 4;
        typedef uint16_t (HANDLER::*mode_method_t)(void);

        /*
        ** HANDLER Class
        */
        class HANDLER
        {
        public:
            friend class SEGMENT;

            HANDLER(pixel_order_t order, uint16_t count)
                : _grey(COLOR::GREY_MODE::LUMINANCE), _mode(EZ_PIXEL_DEFAULT_MODE), _state(EZ_PIXEL_STATE_OFF),
                  _level(EZ_PIXEL_DEFAULT_LEVEL), _speed(EZ_PIXEL_DEFAULT_SPEED), _rOffset(0), _gOffset(0), _bOffset(0),
                  _wOffset(0), _pixelLevel(0), _pixelData(nullptr), _pixelOrder(order), _pixelCount(0), _pixelBits(0),
                  _pixelSize(0), _isDirty(false), _triggered(false)
            {
                if (count)
                {
                    __pixelOrder(order);
                    __pixelCount(count);
                    clear();
                }

                _colors[0] = EZ_COLOR_WHITE;
                _colors[1] = EZ_COLOR_GREEN;
                _colors[2] = EZ_COLOR_BLUE;
                _colors[3] = EZ_COLOR_RED;

                _modeMethods[0] = &HANDLER::_modeDummy;
                _modeMethods[1] = &HANDLER::_modeStatic;
                _modeMethods[2] = &HANDLER::_modeBreathing;
                _modeMethods[3] = &HANDLER::_modeBlink;
                _modeMethods[4] = &HANDLER::_modeBlinkDuo;
                _modeMethods[5] = &HANDLER::_modeBlinkRainbow;
                _modeMethods[6] = &HANDLER::_modeStrobe;
                _modeMethods[7] = &HANDLER::_modeStrobeDuo;
                _modeMethods[8] = &HANDLER::_modeStrobeRainbow;
                _modeMethods[9] = &HANDLER::_modeMultiStrobe;
                _modeMethods[10] = &HANDLER::_modeColorWipe;
                _modeMethods[11] = &HANDLER::_modeColorWipeInv;
                _modeMethods[12] = &HANDLER::_modeColorWipeRev;
                _modeMethods[13] = &HANDLER::_modeColorWipeInvRev;
                _modeMethods[14] = &HANDLER::_modeColorWipeRandom;
                _modeMethods[15] = &HANDLER::_modeColorSweepRandom;
                _modeMethods[16] = &HANDLER::_modeRandomColor;
                _modeMethods[17] = &HANDLER::_modeSingleDynamic;
                _modeMethods[18] = &HANDLER::_modeMultiDynamic;
                _modeMethods[19] = &HANDLER::_modeRainbow;
                _modeMethods[20] = &HANDLER::_modeRainbowCycle;
                _modeMethods[21] = &HANDLER::_modeFader;
                _modeMethods[22] = &HANDLER::_modeTheaterChase;
                _modeMethods[23] = &HANDLER::_modeTheaterChaseRainbow;
                _modeMethods[24] = &HANDLER::_modeTwinkle;
                _modeMethods[25] = &HANDLER::_modeTwinkleRandom;
                _modeMethods[26] = &HANDLER::_modeTwinkleFade;
                _modeMethods[27] = &HANDLER::_modeTwinkleFadeRandom;
                _modeMethods[28] = &HANDLER::_modeSparkle;
                _modeMethods[29] = &HANDLER::_modeFlashSparkle;
                _modeMethods[30] = &HANDLER::_modeHyperSparkle;
                _modeMethods[31] = &HANDLER::_modeRunningLights;
                _modeMethods[32] = &HANDLER::_modeRunningRandom;
                _modeMethods[33] = &HANDLER::_modeRunningColor;
                _modeMethods[34] = &HANDLER::_modeRunningEmergency;
                _modeMethods[35] = &HANDLER::_modeRunningHalloween;
                _modeMethods[36] = &HANDLER::_modeRunningChristmas;
                _modeMethods[37] = &HANDLER::_modeScan;
                _modeMethods[38] = &HANDLER::_modeDualScan;
                _modeMethods[39] = &HANDLER::_modeLarsonScanner;
                _modeMethods[40] = &HANDLER::_modeLarsonRainbow;
                _modeMethods[41] = &HANDLER::_modeICU;
                _modeMethods[42] = &HANDLER::_modeChaseWhite;
                _modeMethods[43] = &HANDLER::_modeChaseColor;
                _modeMethods[44] = &HANDLER::_modeChaseRandom;
                _modeMethods[45] = &HANDLER::_modeChaseRainbowWhite;
                _modeMethods[46] = &HANDLER::_modeChaseWhiteRainbow;
                _modeMethods[47] = &HANDLER::_modeChaseBlack;
                _modeMethods[48] = &HANDLER::_modeChaseRainbowBlack;
                _modeMethods[49] = &HANDLER::_modeChaseFlash;
                _modeMethods[50] = &HANDLER::_modeChaseFlashRandom;
                _modeMethods[51] = &HANDLER::_modeBiColorChase;
                _modeMethods[52] = &HANDLER::_modeTriColorChase;
                _modeMethods[53] = &HANDLER::_modeCircusCombustus;
                _modeMethods[54] = &HANDLER::_modeComet;
                _modeMethods[55] = &HANDLER::_modeFireworks;
                _modeMethods[56] = &HANDLER::_modeFireworksRandom;
                _modeMethods[57] = &HANDLER::_modeFireFlicker;
                _modeMethods[58] = &HANDLER::_modeFireFlickerSoft;
                _modeMethods[59] = &HANDLER::_modeFireFlickerIntense;

                _reset();
            }

            virtual ~HANDLER()
            {
                if (_pixelData)
                {
                    _pixelData = _memFree(_pixelData);
                }
            }

            bool service(void)
            {
                bool _isFrame = false;

                if (_state != EZ_PIXEL_STATE_OFF || _triggered)
                {
                    // Be aware, millis() rolls over every 49 days
                    unsigned long now = millis();
                    uint16_t delay = 0;

                    if (now > _next_time || _triggered)
                    {
                        _isFrame = true;
                        _isCycle = false;

                        if (_state == EZ_PIXEL_STATE_TURNING_OFF)
                        {
                            uint8_t level;

                            if ((level = getPixelLevel()) == 0)
                            {
                                _state = EZ_PIXEL_STATE_OFF;
                                _isCycle = true;
                                clear();
                            }
                            else
                            {
                                setPixelLevel(level - 1);
                                delay = EZ_PIXEL_ONOFF_DELAY;
                            }
                        }
                        else if (_state == EZ_PIXEL_STATE_TURNING_ON)
                        {
                            if (getPixelLevel() == _level)
                            {
                                _state = EZ_PIXEL_STATE_ON;
                                _isCycle = true;
                            }
                            else
                            {
                                fill(_colors[0]);
                                _pixelLevel++;
                                delay = EZ_PIXEL_ONOFF_DELAY;
                            }
                        }
                        else
                        {
                            delay = (this->*_modeMethods[_mode])();
                            _counter_mode_call++;
                        }

                        _next_time = now + max(delay, EZ_PIXEL_SPEED_MIN);
                        _triggered = false;
                    }
                }

                return _isFrame;
            }

            bool isCycle(void) { return _isCycle; }
            void trigger(void) { _triggered = true; }

            // Set mode
            //
            // mode = 0 : off
            // mode = 1 : On (static)
            // mode +   : FX
            //
            uint8_t getState(void) { return _state; }
            uint8_t getMode(void) { return _mode; }

            void setMode(uint8_t mode, uint8_t options = EZ_PIXEL_OPTION_NONE)
            {
                if (mode == 0 && _mode != 0 && _state != EZ_PIXEL_STATE_OFF)
                {
                    _state = EZ_PIXEL_STATE_TURNING_OFF;
                }
                else if (mode == 1 && _state == EZ_PIXEL_STATE_OFF)
                {
                    _state = EZ_PIXEL_STATE_TURNING_ON;
                    _pixelLevel = 1;
                    clear();
                }
                else if (mode != _mode && mode < MODE_COUNT)
                {
                    _state = EZ_PIXEL_STATE_ON;
                    setLevel(_level);
                    clear();
                }
                else
                    return;

                _mode = constrain(mode, 0, MODE_COUNT - 1);
                _reset();
                _options = options;
            }

            uint8_t getModeCount(void) { return MODE_COUNT; }
            const __FlashStringHelper* getModeName(void) { return getModeName(_mode); }
            const __FlashStringHelper* getModeName(uint8_t m)
            {
                if (m < MODE_COUNT)
                {
                    return modeNames[m];
                }
                else
                {
                    return F("");
                }
            }

            // Grey mode
            //
            COLOR::GREY_MODE getGreyMode(void) { return _grey; }
            void setGreyMode(COLOR::GREY_MODE mode) { _grey = mode; }

            // Pixel Level (Brightness)
            //
            uint8_t getLevel(void) { return _level; }

            void setLevel(uint8_t l)
            {
                _level = constrain(l, EZ_PIXEL_LEVEL_MIN, EZ_PIXEL_LEVEL_MAX);
                setPixelLevel(_level);
            }

            void increaseLevel(uint8_t s) { setLevel(constrain(_level + s, EZ_PIXEL_LEVEL_MIN, EZ_PIXEL_LEVEL_MAX)); }

            void decreaseLevel(uint8_t s) { setLevel(constrain(_level - s, EZ_PIXEL_LEVEL_MIN, EZ_PIXEL_LEVEL_MAX)); }

            // FX Speed
            //
            uint16_t getSpeed(void) { return _speed; }

            void setSpeed(uint16_t s)
            {
                _reset();
                _speed = constrain(s, EZ_PIXEL_SPEED_MIN, EZ_PIXEL_SPEED_MAX);
            }

            void increaseSpeed(uint8_t s) { setSpeed(constrain(_speed + s, EZ_PIXEL_SPEED_MIN, EZ_PIXEL_SPEED_MAX)); }

            void decreaseSpeed(uint8_t s) { setSpeed(constrain(_speed - s, EZ_PIXEL_SPEED_MIN, EZ_PIXEL_SPEED_MAX)); }

            // FX Color(s)
            //
            COLOR getColor(uint8_t n)
            {
                if (n < MAX_SFX_COLORS)
                    return _colors[n];
                return EZ_COLOR_BLACK;
            }

            void setColor(uint8_t n, uint8_t m) { setColor(n, COLOR(m, m, m, m)); }
            void setColor(uint8_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { setColor(n, COLOR(r, g, b, w)); }
            void setColor(uint8_t n, COLOR c)
            {
                if (n < MAX_SFX_COLORS)
                {
                    _reset();
                    _colors[n] = c;
                    setLevel(_level);
                }
            }

            // Options
            //
            void setOptions(uint8_t options) { _options = options; }
            uint8_t getOptions(void) { return _options; }

            // Fill with single color
            //
            void fill(uint8_t m) { fill(m, m, m, m); }
            void fill(COLOR c) { fill(c.r, c.g, c.b, c.w); }
            void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0)
            {
                for (int n = 0; n < _pixelCount; n++)
                {
                    setPixel(n, r, g, b, w);
                }
            }

            // Set a single pixel
            //
            void setPixel(uint16_t n, uint8_t m) { setPixel(n, m, m, m, m); }
            void setPixel(uint16_t n, COLOR c) { setPixel(n, c.r, c.g, c.b, c.w); }
            void setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0)
            {
                if ((_pixelData) && n < _pixelCount)
                {
                    uint8_t* p;

                    if (_pixelLevel)
                    {
                        r = (r * _pixelLevel) >> 8;
                        g = (g * _pixelLevel) >> 8;
                        b = (b * _pixelLevel) >> 8;
                        w = (w * _pixelLevel) >> 8;
                    }

                    if (_pixelOrder != ORDER::MONO)
                    {
                        if (_wOffset == _rOffset)
                        {                           // RGB-type strip
                            p = &_pixelData[n * 3]; // 3 bytes per pixel
                        }
                        else
                        {                           // RGBW-type strip
                            p = &_pixelData[n * 4]; // 4 bytes per pixel
                            p[_wOffset] = w;        // Store W
                        }

                        p[_rOffset] = r; // R,G,B always stored
                        p[_gOffset] = g;
                        p[_bOffset] = b;
                    }
                    else
                    {
                        COLOR mono(r, g, b, w);
                        _pixelData[n] = mono.getGrey(_grey);
                    }

                    _setDirty();
                }
            }

            // Get a single pixel
            //
            COLOR getPixel(uint16_t n)
            {
                COLOR c(0);

                if ((_pixelData) && n < _pixelCount)
                {
                    if (_pixelOrder != ORDER::MONO)
                    {
                        uint8_t* p = &_pixelData[n * ((_wOffset == _rOffset) ? 3 : 4)];

                        if (_pixelLevel)
                        {
                            c.r = (p[_rOffset] << 8) / _pixelLevel;
                            c.g = (p[_gOffset] << 8) / _pixelLevel;
                            c.b = (p[_bOffset] << 8) / _pixelLevel;
                            if (_wOffset != _rOffset)
                                c.w = (p[_wOffset] << 8) / _pixelLevel;
                        }
                        else
                        {
                            c.r = p[_rOffset];
                            c.g = p[_gOffset];
                            c.b = p[_bOffset];
                            if (_wOffset != _rOffset)
                                c.w = p[_wOffset];
                        }
                    }
                    else
                    {
                        c.r = c.g = c.b = c.w = (_pixelLevel ? (_pixelData[n] << 8) / _pixelLevel : _pixelData[n]);
                    }
                }

                return c;
            }

            // Change Pixel Levels
            //
            void setPixelLevel(uint8_t level)
            {
                // Stored brightness level is different than what's passed.
                // This simplifies the actual scaling math later, allowing a fast
                // 8x8-bit multiply and taking the MSB.  'level' is a uint8_t,
                // adding 1 here may (intentionally) roll over...so 0 = max level
                // (color values are interpreted literally; no scaling),
                //
                // 1 = min brightness level (off)
                // 255 = just below max brightness level.
                //
                uint8_t newLevel = level + 1;

                if ((_pixelData) && newLevel != _pixelLevel)
                {
                    // Level has changed -- re-scale existing data in RAM
                    uint8_t c, *ptr = (uint8_t*)_pixelData, oldLevel = _pixelLevel - 1;
                    uint16_t scale;

                    if (oldLevel == 0)
                        scale = 0; // Avoid /0
                    else if (level == 255)
                        scale = 65535 / oldLevel;
                    else
                        scale = (((uint16_t)newLevel << 8) - 1) / oldLevel;

                    for (uint16_t i = 0; i < _pixelBytes; i++)
                    {
                        c = *ptr;
                        *ptr++ = (c * scale) >> 8;
                    }

                    _pixelLevel = newLevel;
                    _setDirty();
                }
            }

            uint8_t getPixelLevel(void) { return _pixelLevel - 1; }

            // Blackout!
            //
            void clear(void)
            {
                if (_pixelData)
                    memset(_pixelData, 0, _pixelBytes);
                _setDirty();
            }

            uint16_t pixels(void) { return _pixelCount; }

        protected:
            COLOR::GREY_MODE _grey;
            uint8_t _mode;
            uint8_t _state;
            uint8_t _level;
            uint16_t _speed;
            uint8_t _rOffset;
            uint8_t _gOffset;
            uint8_t _bOffset;
            uint8_t _wOffset;
            uint8_t _pixelLevel;
            uint8_t* _pixelData;
            pixel_order_t _pixelOrder;
            uint16_t _pixelCount;
            size_t _pixelBytes;
            size_t _pixelBits;
            size_t _pixelSize;
            bool _isDirty;
            bool _isCycle;
            bool _triggered;

            COLOR _colors[MAX_SFX_COLORS];

            uint8_t _options;
            unsigned long _next_time;
            uint32_t _counter_mode_step;
            uint32_t _counter_mode_call;
            uint8_t _aux_param1;  // auxilary param (usually stores a color_wheel index)
            uint16_t _aux_param2; // auxilary param (usually stores a segment index)

            mode_method_t _modeMethods[MODE_COUNT];

            inline virtual void _setDirty(void) { _isDirty = true; }

            virtual uint8_t* _memAlloc(size_t size) { return (uint8_t*)malloc(size); }
            virtual uint8_t* _memFree(uint8_t* mem)
            {
                free((void*)mem);
                return nullptr;
            }

            void _reset()
            {
                _isCycle = false;
                _next_time = 0;
                _aux_param1 = 0;
                _aux_param2 = 0;
                _counter_mode_call = _counter_mode_step = 0;
            }

            /**************
            ** Mode FX's **
            **************/

            uint16_t _modeDummy(void) { return 500; }

            uint16_t _modeStatic(void)
            {
                fill(_colors[0]);
                _isCycle = true;
                return 500;
            }

            // Does the "standby-breathing" of well known i-Devices. Fixed Speed.
            // Use mode "modeFader" if you like to have something similar with
            // a different speed.
            //
            uint16_t _modeBreathing(void)
            {
                int lum = _counter_mode_step;

                if (lum > 255)
                    lum = 511 - lum; // lum = 15 -> 255 -> 15

                uint16_t delay;

                if (lum == 15)
                {
                    delay = 970; // 970 pause before each breath
                    _isCycle = true;
                }
                else if (lum <= 25)
                    delay = 38; // 19
                else if (lum <= 50)
                    delay = 36; // 18
                else if (lum <= 75)
                    delay = 28; // 14
                else if (lum <= 100)
                    delay = 20; // 10
                else if (lum <= 125)
                    delay = 14; // 7
                else if (lum <= 150)
                    delay = 11; // 5
                else
                    delay = 10; // 4

                COLOR color = _colors[1];

                color.r = color.r * lum / 256;
                color.g = color.g * lum / 256;
                color.b = color.b * lum / 256;
                color.w = color.w * lum / 256;

                fill(color);

                _counter_mode_step += 2;

                if (_counter_mode_step > (512 - 15))
                    _counter_mode_step = 15;

                return delay;
            }

            // Blinking and Strobes
            //
            uint16_t _modeBlink(void) { return _helpBlink(_colors[1], 0, false); }
            uint16_t _modeBlinkDuo(void) { return _helpBlink(_colors[2], _colors[1], false); }
            uint16_t _modeBlinkRainbow(void) { return _helpBlink(_colorWheel(_counter_mode_call & 0xFF), 0, false); }
            uint16_t _modeStrobe(void) { return _helpBlink(_colors[1], 0, true); }
            uint16_t _modeStrobeDuo(void) { return _helpBlink(_colors[2], _colors[1], true); }
            uint16_t _modeStrobeRainbow(void) { return _helpBlink(_colorWheel(_counter_mode_call & 0xFF), 0, true); }
            uint16_t _modeMultiStrobe(void)
            {
                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    setPixel(i, EZ_COLOR_BLACK);
                }

                uint16_t delay = 200 + ((9 - (_speed % 10)) * 100);
                uint16_t count = 2 * ((_speed / 100) + 1);

                if (_counter_mode_step < count)
                {
                    if ((_counter_mode_step & 1) == 0)
                    {
                        for (uint16_t i = 0; i < _pixelCount; i++)
                        {
                            setPixel(i, _colors[1]);
                        }
                        delay = 20;
                    }
                    else
                    {
                        delay = 50;
                    }
                }

                _counter_mode_step = (_counter_mode_step + 1) % (count + 1);
                return delay;
            }

            // Color Wipes
            //
            uint16_t _modeColorWipe(void) { return _helpColorWipe(_colors[1], _colors[2], false); }
            uint16_t _modeColorWipeInv(void) { return _helpColorWipe(_colors[2], _colors[1], false); }
            uint16_t _modeColorWipeRev(void) { return _helpColorWipe(_colors[1], _colors[2], true); }
            uint16_t _modeColorWipeInvRev(void) { return _helpColorWipe(_colors[2], _colors[1], true); }
            uint16_t _modeColorWipeRandom(void)
            {
                if (_counter_mode_step % _pixelCount == 0)
                {
                    _aux_param1 = _randomWheelIndex(_aux_param1);
                }

                COLOR color = _colorWheel(_aux_param1);
                return _helpColorWipe(color, color, false) * 2;
            }

            // Random color introduced alternating from start and end of strip.
            //
            uint16_t _modeColorSweepRandom(void)
            {
                if (_counter_mode_step % _pixelCount == 0)
                {
                    _aux_param1 = _randomWheelIndex(_aux_param1);
                }

                COLOR color = _colorWheel(_aux_param1);
                return _helpColorWipe(color, color, true) * 2;
            }

            // Lights all LEDs in one random color up. Then switches them
            // to the next random color.
            //
            uint16_t _modeRandomColor(void)
            {
                _aux_param1 = _randomWheelIndex(_aux_param1);
                COLOR color = _colorWheel(_aux_param1);
                fill(color);
                _isCycle = true;
                return _speed;
            }

            // Lights every LED in a random color. Changes one random LED after the other
            // to another random color.
            //
            uint16_t _modeSingleDynamic(void)
            {
                if (_counter_mode_call == 0)
                {
                    for (uint16_t i = 0; i < _pixelCount; i++)
                    {
                        setPixel(i, _colorWheel(_random8()));
                    }
                }

                setPixel(random(_pixelCount), _colorWheel(_random8()));
                _isCycle = true;
                return (_speed);
            }

            // Lights every LED in a random color. Changes all LED at the same time
            // to new random colors.
            //
            uint16_t _modeMultiDynamic(void)
            {
                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    setPixel(i, _colorWheel(_random8()));
                }
                _isCycle = true;
                return (_speed);
            }

            // Cycles all LEDs at once through a rainbow.
            //
            uint16_t _modeRainbow(void)
            {
                COLOR color = _colorWheel(_counter_mode_step);
                fill(color);
                if ((_counter_mode_step = (_counter_mode_step + 1) & 0xFF) == 0xFF)
                    _isCycle = true;
                return (_speed / 256);
            }

            // Cycles a rainbow over the entire string of LEDs.
            //
            uint16_t _modeRainbowCycle(void)
            {
                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    COLOR color = _colorWheel(((i * 256 / _pixelCount) + _counter_mode_step) & 0xFF);
                    setPixel(i, color);
                }

                if ((_counter_mode_step = (_counter_mode_step + 1) & 0xFF) == 0xFF)
                    _isCycle = true;
                return (_speed / 256);
            }

            // Fades the LEDs between two colors
            //
            uint16_t _modeFader(void)
            {
                int lum = _counter_mode_step;

                if (lum > 255)
                    lum = 511 - lum; // lum = 0 -> 255 -> 0

                fill(_helpBlend(_colors[1], _colors[2], lum));

                _counter_mode_step += 4;
                if (_counter_mode_step > 511)
                {
                    _counter_mode_step = 0;
                    _isCycle = true;
                }
                return (_speed / 128);
            }

            // Theatre-style crawling lights.
            //
            uint16_t _modeTheaterChase(void) { return _helpTheaterChase(_colors[1], _colors[2]); }

            // Theatre-style crawling lights with rainbow effect.
            //
            uint16_t _modeTheaterChaseRainbow(void)
            {
                _counter_mode_step = (_counter_mode_step + 1) & 0xFF;
                return _helpTheaterChase(_colorWheel(_counter_mode_step), EZ_COLOR_BLACK);
            }

            // Blink several LEDs on, reset, repeat.
            // Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
            //
            uint16_t _modeTwinkle(void) { return _helpTwinkle(_colors[1], _colors[2]); }

            // Blink several LEDs in random colors on, reset, repeat.
            // Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
            //
            uint16_t _modeTwinkleRandom(void) { return _helpTwinkle(_colorWheel(_random8()), _colors[2]); }

            // Blink several LEDs on, fading out.
            //
            uint16_t _modeTwinkleFade(void) { return _helpTwinkleFade(_colors[1]); }

            // Blink several LEDs in random colors on, fading out.
            //
            uint16_t _modeTwinkleFadeRandom(void) { return _helpTwinkleFade(_colorWheel(_random8())); }

            // Blinks one LED at a time.
            // Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
            //
            uint16_t _modeSparkle(void)
            {
                setPixel(_aux_param2, EZ_COLOR_BLACK);
                _aux_param2 = random(_pixelCount); // aux_param3 stores the random led index
                setPixel(_aux_param2, _colors[1]);
                return (_speed / _pixelCount);
            }

            // Lights all LEDs in the color. Flashes single white pixels randomly.
            // Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
            //
            uint16_t _modeFlashSparkle(void)
            {
                if (_counter_mode_call == 0)
                {
                    for (uint16_t i = 0; i < _pixelCount; i++)
                    {
                        setPixel(i, _colors[1]);
                    }
                }

                setPixel(_aux_param2, _colors[1]);

                if (_random8(5) == 0)
                {
                    _aux_param2 = random(_pixelCount);
                    setPixel(_aux_param2, EZ_COLOR_WHITE);
                    return 20;
                }

                return _speed;
            }

            // Like flash sparkle. With more flash.
            // Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
            //
            uint16_t _modeHyperSparkle(void)
            {
                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    setPixel(i, _colors[1]);
                }

                if (_random8(5) < 2)
                {
                    for (uint16_t i = 0; i < max(1, _pixelCount / 3); i++)
                    {
                        setPixel(random(_pixelCount), EZ_COLOR_WHITE);
                    }
                    return 20;
                }

                return _speed;
            }

            // Running lights effect with smooth sine transition.
            //
            uint16_t _modeRunningLights(void)
            {
                uint8_t sineIncr = max(1, (256 / _pixelCount));
                uint16_t stop = _pixelCount - 1;

                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    int lum = (int)sine8(((i + _counter_mode_step) * sineIncr));

                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(i, (_colors[1].r * lum) / 256, (_colors[1].g * lum) / 256, (_colors[1].b * lum) / 256,
                                 (_colors[1].w * lum) / 256);
                    }
                    else
                    {
                        setPixel(stop - i, (_colors[1].r * lum) / 256, (_colors[1].g * lum) / 256,
                                 (_colors[1].b * lum) / 256, (_colors[1].w * lum) / 256);
                    }
                }

                _counter_mode_step = (_counter_mode_step + 1) % 256;
                return (_speed / _pixelCount);
            }

            // Random colored pixels running.
            //
            uint16_t _modeRunningRandom(void)
            {
                uint16_t stop = _pixelCount - 1;

                for (uint16_t i = stop; i > 0; i--)
                {
                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(stop - i, getPixel(stop - i + 1));
                    }
                    else
                    {
                        setPixel(i, getPixel(i - 1));
                    }
                }

                if (_counter_mode_step == 0)
                {
                    _aux_param1 = _randomWheelIndex(_aux_param1);

                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(stop, _colorWheel(_aux_param1));
                    }
                    else
                    {
                        setPixel(0, _colorWheel(_aux_param1));
                    }
                }

                _counter_mode_step = (_counter_mode_step == 0) ? 1 : 0;
                return (_speed / _pixelCount);
            }

            uint16_t _modeRunningColor(void) { return _helpRunning(_colors[1], EZ_COLOR_WHITE); }
            uint16_t _modeRunningEmergency(void) { return _helpRunning(EZ_COLOR_RED, EZ_COLOR_BLUE); }
            uint16_t _modeRunningHalloween(void) { return _helpRunning(EZ_COLOR_PURPLE, EZ_COLOR_ORANGE); }
            uint16_t _modeRunningChristmas(void) { return _helpRunning(EZ_COLOR_RED, EZ_COLOR_GREEN); }

            // Runs a single pixel back and forth.
            //
            uint16_t _modeScan(void)
            {
                if (_counter_mode_step > (_pixelCount * 2) - 2)
                {
                    _counter_mode_step = 0;
                    _isCycle = true;
                }

                fill(_colors[2]);

                uint16_t stop = _pixelCount - 1;
                uint16_t led_offset = _counter_mode_step - stop;
                led_offset = abs(led_offset);

                if (EZ_PIXEL_IS_REVERSE)
                {
                    setPixel(stop - led_offset, _colors[1]);
                }
                else
                {
                    setPixel(led_offset, _colors[1]);
                }

                _counter_mode_step++;
                return (_speed / (_pixelCount * 2));
            }

            // Runs two pixel back and forth in opposite directions.
            //
            uint16_t _modeDualScan(void)
            {
                if (_counter_mode_step > (_pixelCount * 2) - 2)
                {
                    _counter_mode_step = 0;
                    _isCycle = true;
                }

                fill(_colors[2]);

                uint16_t stop = _pixelCount - 1;
                uint16_t led_offset = _counter_mode_step - stop;
                led_offset = abs(led_offset);

                setPixel(led_offset, _colors[1]);
                setPixel(stop - led_offset, _colors[1]);

                _counter_mode_step++;
                return (_speed / (_pixelCount * 2));
            }

            // K.I.T.T.
            //
            uint16_t _modeLarsonScanner(void)
            {
                _helpFadeOut();
                uint16_t stop = _pixelCount - 1;

                if (_counter_mode_step < _pixelCount)
                {
                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(stop - _counter_mode_step, _colors[1]);
                    }
                    else
                    {
                        setPixel(_counter_mode_step, _colors[1]);
                    }
                }
                else
                {
                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(stop - ((_pixelCount * 2) - _counter_mode_step) + 2, _colors[1]);
                    }
                    else
                    {
                        setPixel(((_pixelCount * 2) - _counter_mode_step) - 2, _colors[1]);
                    }
                }

                _isCycle = (_counter_mode_step % _pixelCount == 0);

                _counter_mode_step = (_counter_mode_step + 1) % ((_pixelCount * 2) - 2);
                return (_speed / (_pixelCount * 2));
            }

            uint16_t _modeLarsonRainbow(void)
            {
                static int16_t dir = 1;

                uint16_t stop = _pixelCount - 1;
                _counter_mode_step += dir;

                _helpFadeOut();

                if (_options == EZ_PIXEL_OPTION_NONE)
                {
                    setPixel(_counter_mode_step, _colorWheel((_counter_mode_call++ % 8) * 32));
                }
                else
                {
                    setPixel(_counter_mode_step, _colorWheel((_counter_mode_step * 256) / _pixelCount));
                }

                if (_counter_mode_step >= stop || _counter_mode_step <= 0)
                    dir = -dir;

                return (_speed / (_pixelCount * 2));
            }

            // ICU mode
            //
            uint16_t _modeICU(void)
            {
                uint16_t dest = _counter_mode_step & 0xFFFF;

                setPixel(dest, _colors[1]);
                setPixel(dest + _pixelCount / 2, _colors[1]);

                if (_aux_param2 == dest)
                {
                    if (_random8(6) == 0)
                    {
                        setPixel(dest, EZ_COLOR_BLACK);
                        setPixel(dest + _pixelCount / 2, EZ_COLOR_BLACK);
                        return 200;
                    }

                    _aux_param2 = random(_pixelCount / 2);
                    return 1000 + random(2000);
                }

                setPixel(dest, EZ_COLOR_BLACK);
                setPixel(dest + _pixelCount / 2, EZ_COLOR_BLACK);

                if (_aux_param2 > _counter_mode_step)
                {
                    _counter_mode_step++;
                    dest++;
                }
                else if (_aux_param2 < _counter_mode_step)
                {
                    _counter_mode_step--;
                    dest--;
                }

                setPixel(dest, _colors[1]);
                setPixel(dest + _pixelCount / 2, _colors[1]);

                return (_speed / _pixelCount);
            }

            uint16_t _modeChaseWhite(void) { return _helpChase(EZ_COLOR_WHITE, _colors[1], _colors[1]); }
            uint16_t _modeChaseColor(void) { return _helpChase(_colors[1], EZ_COLOR_WHITE, EZ_COLOR_WHITE); }
            uint16_t _modeChaseRandom(void)
            {
                if (_counter_mode_step == 0)
                {
                    _aux_param1 = _randomWheelIndex(_aux_param1);
                }
                return _helpChase(_colorWheel(_aux_param1), EZ_COLOR_WHITE, EZ_COLOR_WHITE);
            }

            uint16_t _modeChaseRainbowWhite(void)
            {
                uint8_t color_sep = 256 / _pixelCount;
                uint8_t color_index = _counter_mode_call & 0xFF;
                COLOR color = _colorWheel(((_counter_mode_step * color_sep) + color_index) & 0xFF);

                return _helpChase(color, EZ_COLOR_WHITE, EZ_COLOR_WHITE);
            }

            uint16_t _modeChaseWhiteRainbow(void)
            {
                uint16_t n = _counter_mode_step;
                uint16_t m = (_counter_mode_step + 1) % _pixelCount;
                COLOR color2 = _colorWheel(((n * 256 / _pixelCount) + (_counter_mode_call & 0xFF)) & 0xFF);
                COLOR color3 = _colorWheel(((m * 256 / _pixelCount) + (_counter_mode_call & 0xFF)) & 0xFF);

                return _helpChase(EZ_COLOR_WHITE, color2, color3);
            }

            uint16_t _modeChaseBlack(void) { return _helpChase(_colors[1], EZ_COLOR_BLACK, EZ_COLOR_BLACK); }

            uint16_t _modeChaseRainbowBlack(void)
            {
                uint8_t color_sep = 256 / _pixelCount;
                uint8_t color_index = _counter_mode_call & 0xFF;
                COLOR color = _colorWheel(((_counter_mode_step * color_sep) + color_index) & 0xFF);

                return _helpChase(color, EZ_COLOR_BLACK, EZ_COLOR_BLACK);
            }

            uint16_t _modeChaseFlash(void)
            {
                const static uint8_t flash_count = 4;
                uint8_t flash_step = _counter_mode_call % ((flash_count * 2) + 1);
                uint16_t stop = _pixelCount - 1;

                fill(_colors[1]);
                uint16_t delay = (_speed / _pixelCount);

                if (flash_step < (flash_count * 2))
                {
                    if (flash_step % 2 == 0)
                    {
                        uint16_t n = _counter_mode_step;
                        uint16_t m = (_counter_mode_step + 1) % _pixelCount;

                        if (EZ_PIXEL_IS_REVERSE)
                        {
                            setPixel(stop - n, EZ_COLOR_WHITE);
                            setPixel(stop - m, EZ_COLOR_WHITE);
                        }
                        else
                        {
                            setPixel(n, EZ_COLOR_WHITE);
                            setPixel(m, EZ_COLOR_WHITE);
                        }
                        delay = 20;
                    }
                    else
                    {
                        delay = 30;
                    }
                }
                else
                {
                    _counter_mode_step = (_counter_mode_step + 1) % _pixelCount;
                }

                return delay;
            }

            /*
             * White flashes running, followed by random color.
             */
            uint16_t _modeChaseFlashRandom(void)
            {
                const static uint8_t flash_count = 4;
                uint8_t flash_step = _counter_mode_call % ((flash_count * 2) + 1);

                for (uint16_t i = 0; i < _counter_mode_step; i++)
                {
                    setPixel(i, _colorWheel(_aux_param1));
                }

                uint16_t delay = (_speed / _pixelCount);

                if (flash_step < (flash_count * 2))
                {
                    uint16_t n = _counter_mode_step;
                    uint16_t m = (_counter_mode_step + 1) % _pixelCount;

                    if (flash_step % 2 == 0)
                    {
                        setPixel(n, EZ_COLOR_WHITE);
                        setPixel(+m, EZ_COLOR_WHITE);
                        delay = 20;
                    }
                    else
                    {
                        setPixel(n, _colorWheel(_aux_param1));
                        setPixel(m, EZ_COLOR_BLACK);
                        delay = 30;
                    }
                }
                else
                {
                    _counter_mode_step = (_counter_mode_step + 1) % _pixelCount;

                    if (_counter_mode_step == 0)
                    {
                        _aux_param1 = _randomWheelIndex(_aux_param1);
                    }
                }

                return delay;
            }

            uint16_t _modeBiColorChase(void) { return _helpChase(_colors[1], _colors[2], _colors[3]); }
            uint16_t _modeTriColorChase(void) { return _helpTriColorChase(_colors[1], _colors[2], _colors[3]); }
            uint16_t _modeCircusCombustus(void)
            {
                return _helpTriColorChase(EZ_COLOR_RED, EZ_COLOR_WHITE, EZ_COLOR_BLACK);
            }

            // Firing comets from one end.
            //
            uint16_t _modeComet(void)
            {
                _helpFadeOut();

                if (EZ_PIXEL_IS_REVERSE)
                {
                    uint16_t stop = _pixelCount - 1;
                    setPixel(stop - _counter_mode_step, _colors[1]);
                }
                else
                {
                    setPixel(_counter_mode_step, _colors[1]);
                }

                _counter_mode_step = (_counter_mode_step + 1) % _pixelCount;
                return (_speed / _pixelCount);
            }

            // Fireworks
            //
            uint16_t _modeFireworks(void) { return _helpFireworks(_colors[1]); }
            uint16_t _modeFireworksRandom(void) { return _helpFireworks(_colorWheel(_random8())); }

            // Flickering Fire.
            //
            uint16_t _modeFireFlicker(void) { return _helpFireFlicker(3); }
            uint16_t _modeFireFlickerSoft(void) { return _helpFireFlicker(6); }
            uint16_t _modeFireFlickerIntense(void) { return _helpFireFlicker(1.7); }

            /*****************
            ** Mode Helpers **
            *****************/

            // Fireworks helper
            //
            uint16_t _helpFireworks(COLOR color)
            {
                _helpFadeOut();

                // the new way, manipulate the Adafruit_NeoPixels pixels[] array directly, about 5x faster
                uint8_t* pixels = _pixelData;
                uint8_t pixelsPerLed = (_wOffset == _rOffset) ? 3 : 4;
                uint16_t startPixel = pixelsPerLed + pixelsPerLed;
                uint16_t stopPixel = (_pixelCount - 1) * pixelsPerLed;


                for (uint16_t i = startPixel; i < stopPixel; i++)
                {
                    uint16_t tmpPixel = (pixels[i - pixelsPerLed] >> 2) + pixels[i] + (pixels[i + pixelsPerLed] >> 2);
                    pixels[i] = tmpPixel > 255 ? 255 : tmpPixel;
                }

                if (!_triggered)
                {
                    for (uint16_t i = 0; i < max(1, _pixelCount / 20); i++)
                    {
                        if (_random8(10) == 0)
                        {
                            setPixel(random(_pixelCount), color);
                        }
                    }
                }
                else
                {
                    for (uint16_t i = 0; i < max(1, _pixelCount / 10); i++)
                    {
                        setPixel(random(_pixelCount), color);
                    }
                }
                return (_speed / _pixelCount);
            }

            // Fire flicker helper
            //
            uint16_t _helpFireFlicker(int rev_intensity)
            {
                COLOR color = _colors[1];
                byte lum = max(color.w, max(color.r, max(color.g, color.b))) / rev_intensity;

                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    int flicker = _random8(lum);

                    setPixel(i, max(color.r - flicker, 0), max(color.g - flicker, 0), max(color.b - flicker, 0),
                             max(color.w - flicker, 0));
                }

                return (_speed / _pixelCount);
            }

            // Tricolor chase helper
            //
            uint16_t _helpTriColorChase(COLOR color1, COLOR color2, COLOR color3)
            {
                uint16_t index = _counter_mode_step % 6;
                uint16_t stop = _pixelCount - 1;

                for (uint16_t i = 0; i < _pixelCount; i++, index++)
                {
                    if (index > 5)
                        index = 0;

                    COLOR color = color3;

                    if (index < 2)
                        color = color1;
                    else if (index < 4)
                        color = color2;

                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(i, color);
                    }
                    else
                    {
                        setPixel(stop - i, color);
                    }
                }

                _counter_mode_step++;
                return (_speed / _pixelCount);
            }

            // Running Helper - Alternating pixels
            //
            uint16_t _helpRunning(COLOR color1, COLOR color2)
            {
                uint16_t stop = _pixelCount - 1;

                for (uint16_t i = 0; i <= stop; i++)
                {
                    if ((i + _counter_mode_step) % 4 < 2)
                    {
                        if (EZ_PIXEL_IS_REVERSE)
                        {
                            setPixel(i, color1);
                        }
                        else
                        {
                            setPixel(stop - i, color1);
                        }
                    }
                    else
                    {
                        if (EZ_PIXEL_IS_REVERSE)
                        {
                            setPixel(i, color2);
                        }
                        else
                        {
                            setPixel(stop - i, color2);
                        }
                    }
                }

                _counter_mode_step = (_counter_mode_step + 1) & 0x3;
                return (_speed / _pixelCount);
            }

            // Blink/Strobe Helper
            //
            uint16_t _helpBlink(COLOR color1, COLOR color2, bool strobe)
            {
                COLOR color = ((_counter_mode_call & 1) == 0) ? color1 : color2;

                if (EZ_PIXEL_IS_REVERSE)
                    color = (color == color1) ? color2 : color1;

                fill(color);

                if ((_counter_mode_call & 1) == 0)
                {
                    _isCycle = true;
                    return strobe ? 20 : (_speed / 2);
                }
                else
                {
                    return strobe ? _speed - 20 : (_speed / 2);
                }
            }

            /*
             * Color wipe function
             * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
             * if (bool rev == true) then LEDs are turned off in reverse order
             */
            uint16_t _helpColorWipe(COLOR color1, COLOR color2, bool rev)
            {
                uint16_t stop = _pixelCount - 1;

                if (_counter_mode_step < _pixelCount)
                {
                    uint16_t led_offset = _counter_mode_step;

                    if (EZ_PIXEL_IS_REVERSE)
                    {
                        setPixel(stop - led_offset, color1);
                    }
                    else
                    {
                        setPixel(led_offset, color1);
                    }
                }
                else
                {
                    uint16_t led_offset = _counter_mode_step - _pixelCount;

                    if ((EZ_PIXEL_IS_REVERSE && !rev) || (!EZ_PIXEL_IS_REVERSE && rev))
                    {
                        setPixel(stop - led_offset, color2);
                    }
                    else
                    {
                        setPixel(led_offset, color2);
                    }
                }

                if (_counter_mode_step % _pixelCount == 0)
                    _isCycle = true;

                _counter_mode_step = (_counter_mode_step + 1) % (_pixelCount * 2);

                return (_speed / (_pixelCount * 2));
            }

            // Color blend helpern
            //
            COLOR _helpBlend(COLOR color1, COLOR color2, uint8_t blend)
            {
                if (blend == 0)
                    return color1;
                if (blend == 255)
                    return color2;

                COLOR color3;

                color3.w = ((color2.w * blend) + (color1.w * (255 - blend))) / 256;
                color3.r = ((color2.r * blend) + (color1.r * (255 - blend))) / 256;
                color3.g = ((color2.g * blend) + (color1.g * (255 - blend))) / 256;
                color3.b = ((color2.b * blend) + (color1.b * (255 - blend))) / 256;

                return color3;
            }

            // Theater chase function
            //
            uint16_t _helpTheaterChase(COLOR color1, COLOR color2)
            {
                _counter_mode_call = _counter_mode_call % 3;
                uint16_t stop = _pixelCount - 1;

                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    if ((i % 3) == _counter_mode_call)
                    {
                        if (EZ_PIXEL_IS_REVERSE)
                        {
                            setPixel(stop - i, color1);
                        }
                        else
                        {
                            setPixel(i, color1);
                        }
                    }
                    else
                    {
                        if (EZ_PIXEL_IS_REVERSE)
                        {
                            setPixel(stop - i, color2);
                        }
                        else
                        {
                            setPixel(i, color2);
                        }
                    }
                }

                return (_speed / _pixelCount);
            }

            // Twinkle Helper
            //
            uint16_t _helpTwinkle(COLOR color1, COLOR color2)
            {
                if (_counter_mode_step == 0)
                {
                    for (uint16_t i = 0; i < _pixelCount; i++)
                    {
                        setPixel(i, color2);
                    }

                    uint16_t min_leds = max(1, _pixelCount / 5); // make sure, at least one LED is on
                    uint16_t max_leds = max(1, _pixelCount / 2); // make sure, at least one LED is on
                    _counter_mode_step = random(min_leds, max_leds);
                }

                setPixel(random(_pixelCount), color1);

                _counter_mode_step--;
                return (_speed / _pixelCount);
            }

            // Twinkle Fade Helper
            //
            uint16_t _helpTwinkleFade(COLOR color1, COLOR color2 = EZ_COLOR_BLACK)
            {
                _helpFadeOut(color2);

                if (_random8(3) == 0)
                {
                    setPixel(random(_pixelCount), color1);
                }

                return (_speed / 8);
            }

            // Color chase helper.
            // color1 = background color
            // color2 and color3 = colors of two adjacent leds
            //
            uint16_t _helpChase(COLOR color1, COLOR color2, COLOR color3)
            {
                uint16_t stop = _pixelCount - 1;
                uint16_t a = _counter_mode_step;
                uint16_t b = (a + 1) % _pixelCount;
                uint16_t c = (b + 1) % _pixelCount;

                if (EZ_PIXEL_IS_REVERSE)
                {
                    setPixel(stop - a, color1);
                    setPixel(stop - b, color2);
                    setPixel(stop - c, color3);
                }
                else
                {
                    setPixel(a, color1);
                    setPixel(b, color2);
                    setPixel(c, color3);
                }

                if (b == 0)
                    _isCycle = true;
                else
                    _isCycle = false;

                _counter_mode_step = (_counter_mode_step + 1) % _pixelCount;
                return (_speed / _pixelCount);
            }

            /*
             * fade out function
             */
            void _helpFadeOut(COLOR target = EZ_COLOR_BLACK)
            {
                static const uint8_t rateMapH[] = {0, 1, 1, 1, 2, 3, 4, 6};
                static const uint8_t rateMapL[] = {0, 2, 3, 8, 8, 8, 8, 8};

                uint8_t rate = EZ_PIXEL_OPTION_FADE_RATE;
                uint8_t rateH = rateMapH[rate];
                uint8_t rateL = rateMapL[rate];

                for (uint16_t i = 0; i < _pixelCount; i++)
                {
                    COLOR color = getPixel(i);

                    if (rate == 0)
                    {
                        // old fade-to-black algorithm
                        // setPixel(i, (color.u >> 1) & 0x7F7F7F7F);

                        color.r >>= 1 & 0x7F;
                        color.g >>= 1 & 0x7F;
                        color.b >>= 1 & 0x7F;
                        color.w >>= 1 & 0x7F;

                        setPixel(i, color);
                    }
                    else
                    {
                        // new fade-to-color algorithm
                        // calculate the color differences between the current and target colors
                        uint8_t wdelta = target.w - color.w;
                        uint8_t rdelta = target.r - color.r;
                        uint8_t gdelta = target.g - color.g;
                        uint8_t bdelta = target.b - color.b;

                        // if the current and target colors are almost the same, jump right to the target color,
                        // otherwise calculate an intermediate color. (fixes rounding issues)
                        wdelta = abs(wdelta) < 3 ? wdelta : (wdelta >> rateH) + (wdelta >> rateL);
                        rdelta = abs(rdelta) < 3 ? rdelta : (rdelta >> rateH) + (rdelta >> rateL);
                        gdelta = abs(gdelta) < 3 ? gdelta : (gdelta >> rateH) + (gdelta >> rateL);
                        bdelta = abs(bdelta) < 3 ? bdelta : (bdelta >> rateH) + (bdelta >> rateL);

                        setPixel(i, color.r + rdelta, color.g + gdelta, color.b + bdelta, color.w + wdelta);
                    }
                }
            }

            // Color Wheel
            //
            COLOR _colorWheel(uint8_t pos)
            {
                pos = 255 - pos;
                if (pos < 85)
                {
                    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
                }
                else if (pos < 170)
                {
                    pos -= 85;
                    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
                }
                else
                {
                    pos -= 170;
                    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
                }
            }

            // fast 8-bit random number generator shamelessly borrowed from FastLED
            uint8_t _random8(void)
            {
                _rand16seed = (_rand16seed * 2053) + 13849;
                return (uint8_t)((_rand16seed + (_rand16seed >> 8)) & 0xFF);
            }

            // note random8(uint8_t) generates numbers in the range 0 - 254, 255 is never generated
            uint8_t _random8(uint8_t lim)
            {
                uint8_t r = _random8();
                r = (r * lim) >> 8;
                return r;
            }

            uint8_t _randomWheelIndex(uint8_t pos)
            {
                uint8_t r = 0;
                uint8_t x = 0;
                uint8_t y = 0;
                uint8_t d = 0;

                while (d < 42)
                {
                    r = _random8();
                    x = abs(pos - r);
                    y = 255 - x;
                    d = min(x, y);
                }

                return r;
            }

        private:
            uint16_t _rand16seed;

            HANDLER(HANDLER const& copy);            // Not Implemented
            HANDLER& operator=(HANDLER const& copy); // Not Implemented

            void __pixelOrder(pixel_order_t t)
            {
                if ((_pixelOrder = t) != ORDER::MONO)
                {
                    bool oldThreeBytesPerPixel = (_wOffset == _rOffset); // false if RGBW

                    _wOffset = (t >> 6) & 0b11;
                    _rOffset = (t >> 4) & 0b11;
                    _gOffset = (t >> 2) & 0b11;
                    _bOffset = t & 0b11;

                    // If bytes-per-pixel has changed (and pixel data was previously
                    // allocated), re-allocate to new size.  Will clear any data.
                    bool newThreeBytesPerPixel = (_wOffset == _rOffset);

                    if (_pixelData)
                    {
                        if (newThreeBytesPerPixel != oldThreeBytesPerPixel)
                            __pixelCount(_pixelCount);
                    }
                }
                else
                {
                    _wOffset = _rOffset = _gOffset = _bOffset = 0;

                    if (_pixelData)
                        __pixelCount(_pixelCount);
                }
            }

            void __pixelCount(uint16_t n)
            {
                if (_pixelData)
                {
                    _pixelData = _memFree(_pixelData);
                }

                if (_pixelOrder != ORDER::MONO)
                {
                    _pixelSize = ((_wOffset == _rOffset) ? 3 : 4);
                }
                else
                    _pixelSize = 1;

                _pixelBits = 8 * _pixelSize;
                _pixelBytes = _pixelSize * n;

                if ((_pixelData = _memAlloc(_pixelBytes)))
                {
                    memset(_pixelData, 0, _pixelBytes);
                    _pixelCount = n;
                }
                else
                {
                    _pixelBytes = 0;
                    _pixelCount = 0;
                }
            }
        };
    } // namespace PIXEL
} // namespace EZ
#endif // _EZ_PIXEL_HANDLER_H