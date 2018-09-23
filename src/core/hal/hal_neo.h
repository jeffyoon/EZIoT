/*
** EZIoT - NEO Pixels
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** Based upon work by Adafruit (NeoPixel Libray) and
** ESP32 RMT Peripheral Driver for WS2812's by Chris Osborn and Martin F. Falatic
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZ_HAL_NEO_H
#define _EZ_HAL_NEO_H
#include "core/tool/ez_color.h"
#include "pixel/pixel_handler.h"
#include "pixel/pixel_segment.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(ARDUINO)
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "esp32-hal.h"
#include "esp_intr.h"
#include "freertos/semphr.h"
#include "soc/rmt_struct.h"
#elif defined(ESP_PLATFORM)
#include <driver/gpio.h>
#include <esp_intr.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <soc/dport_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/rmt_struct.h>
#include <stdio.h>
#endif
#include "driver/rmt.h"

#ifdef __cplusplus
}
#endif

namespace EZ
{
    namespace NEO
    {
        // Forward references
        //
        class STRAND;
        class DRIVER;

        static IRAM_ATTR void __rmtCopyBlock(STRAND* pStrand);
        static IRAM_ATTR void __rmtInterrupt(void* arg);

        // Pixel types and associtaed timings
        //
        typedef enum TYPE
        {
            WS2812_V1,
            WS2812B_V1,
            WS2812B_V2,
            WS2812B_V3,
            WS2813_V1,
            WS2813_V2,
            WS2813_V3,
            SK6812_V1,
            SK6812W_V1
        } neo_type_t;

        typedef struct
        {
            int bytesPerPixel;
            uint32_t T0H;
            uint32_t T1H;
            uint32_t T0L;
            uint32_t T1L;
            uint32_t TRS;
        } neo_params_t;

        static const neo_params_t __neoParams[] = {
            [TYPE::WS2812_V1] = {.bytesPerPixel = 3, .T0H = 350, .T1H = 700, .T0L = 800, .T1L = 600, .TRS = 50000},
            [TYPE::WS2812B_V1] = {.bytesPerPixel = 3, .T0H = 350, .T1H = 900, .T0L = 900, .T1L = 350, .TRS = 50000},
            [TYPE::WS2812B_V2] = {.bytesPerPixel = 3, .T0H = 400, .T1H = 850, .T0L = 850, .T1L = 400, .TRS = 50000},
            [TYPE::WS2812B_V3] = {.bytesPerPixel = 3, .T0H = 450, .T1H = 850, .T0L = 850, .T1L = 450, .TRS = 50000},
            [TYPE::WS2813_V1] = {.bytesPerPixel = 3, .T0H = 350, .T1H = 800, .T0L = 350, .T1L = 350, .TRS = 300000},
            [TYPE::WS2813_V2] = {.bytesPerPixel = 3, .T0H = 270, .T1H = 800, .T0L = 800, .T1L = 270, .TRS = 300000},
            [TYPE::WS2813_V3] = {.bytesPerPixel = 3, .T0H = 270, .T1H = 630, .T0L = 630, .T1L = 270, .TRS = 300000},
            [TYPE::SK6812_V1] = {.bytesPerPixel = 3, .T0H = 300, .T1H = 600, .T0L = 900, .T1L = 600, .TRS = 80000},
            [TYPE::SK6812W_V1] = {.bytesPerPixel = 4, .T0H = 300, .T1H = 600, .T0L = 900, .T1L = 600, .TRS = 80000},
        };

        // LUT for mapping bits in RMT.int_<op>.ch<n>_tx_thr_event
        //
        static DRAM_ATTR const uint32_t __tx_thr_event_offsets[] = {
            static_cast<uint32_t>(1) << (24 + 0), static_cast<uint32_t>(1) << (24 + 1),
            static_cast<uint32_t>(1) << (24 + 2), static_cast<uint32_t>(1) << (24 + 3),
            static_cast<uint32_t>(1) << (24 + 4), static_cast<uint32_t>(1) << (24 + 5),
            static_cast<uint32_t>(1) << (24 + 6), static_cast<uint32_t>(1) << (24 + 7),
        };

        // LUT for mapping bits in RMT.int_<op>.ch<n>_tx_end
        //
        static DRAM_ATTR const uint32_t __tx_end_offsets[] = {
            static_cast<uint32_t>(1) << (0 + 0) * 3, static_cast<uint32_t>(1) << (0 + 1) * 3,
            static_cast<uint32_t>(1) << (0 + 2) * 3, static_cast<uint32_t>(1) << (0 + 3) * 3,
            static_cast<uint32_t>(1) << (0 + 4) * 3, static_cast<uint32_t>(1) << (0 + 5) * 3,
            static_cast<uint32_t>(1) << (0 + 6) * 3, static_cast<uint32_t>(1) << (0 + 7) * 3,
        };

        // A channel has a 64 "pulse" buffer - we use half per pass
        static DRAM_ATTR const uint16_t MAX_PULSES = 32;
        // 8 still seems to work, but timings become marginal
        static DRAM_ATTR const uint16_t DIVIDER = 4;
        // Minimum time of a single RMT duration based on clock ns
        static DRAM_ATTR const double RMT_DURATION_NS = 12.5;
        // In theory upto 8!
        static DRAM_ATTR const uint16_t MAX_STRANDS = 4;

        class RENDER
        {
        public:
            inline void render(STRAND& s) { render(&s); }
            virtual void render(STRAND* pStrand) = 0;
            virtual void render(void) = 0;
        };

        class STRAND : public PIXEL::HANDLER
        {
        public:
            friend class DRIVER;
            friend IRAM_ATTR void __rmtCopyBlock(STRAND* pStrand);
            friend IRAM_ATTR void __rmtInterrupt(void* arg);

            STRAND(PIXEL::pixel_order_t order, uint16_t count, neo_type_t type, uint8_t gpio, rmt_channel_t rmt)
                : HANDLER(order, count), _rmtChannel(rmt), _rmtPin((gpio_num_t)gpio), _neoType(type),
                  _neoRender(nullptr), _rmtSemaphore(nullptr), _rmtIndex(0), _rmtHalf(0), _rmtDirty(false)
            {
            }

            virtual ~STRAND() {}

            void render(void) { if (_neoRender) _neoRender->render(this); }

        protected:
            typedef union {
                struct
                {
                    uint32_t duration0 : 15;
                    uint32_t level0 : 1;
                    uint32_t duration1 : 15;
                    uint32_t level1 : 1;
                };
                uint32_t val;
            } rmt_pulse_t;

            rmt_pulse_t _rmtPulsePair[2];
            rmt_channel_t _rmtChannel;
            gpio_num_t _rmtPin;
            neo_type_t _neoType;
            RENDER* _neoRender;

            volatile xSemaphoreHandle _rmtSemaphore;
            volatile uint16_t _rmtIndex, _rmtHalf;
            volatile bool _rmtDirty;

        private:
            STRAND(STRAND const& copy);            // Not Implemented
            STRAND& operator=(STRAND const& copy); // Not Implemented
        };

        class DRIVER : public RENDER
        {
        public:
            friend class STRAND;
            friend IRAM_ATTR void __rmtCopyBlock(STRAND* pStrand);
            friend IRAM_ATTR void __rmtInterrupt(void* arg);

            DRIVER(STRAND& s1) : _strands{&s1, nullptr, nullptr, nullptr} { _initialise(); }
            DRIVER(STRAND& s1, STRAND& s2) : _strands{&s1, &s2, nullptr, nullptr} { _initialise(); }
            DRIVER(STRAND& s1, STRAND& s2, STRAND& s3) : _strands{&s1, &s2, &s3, nullptr} { _initialise(); }
            DRIVER(STRAND& s1, STRAND& s2, STRAND& s3, STRAND& s4) : _strands{&s1, &s2, &s3, &s4} { _initialise(); }

            ~DRIVER() { stop(); }

            void render(void)
            {
                for (int s = 0; s < MAX_STRANDS; s++)
                {
                    if ((_strands[s]) && _strands[s]->_isDirty)
                        render(_strands[s]);
                }
            }

            void render(STRAND* pStrand)
            {
                if ((pStrand) && pStrand->_isDirty)
                {
                    pStrand->_rmtIndex = 0;
                    pStrand->_rmtHalf = 0;
                    pStrand->_rmtDirty = 1;

                    __rmtCopyBlock(pStrand);
                    if (pStrand->_rmtIndex < pStrand->_pixelBytes)
                        __rmtCopyBlock(pStrand);

                    pStrand->_rmtSemaphore = xSemaphoreCreateBinary();

                    RMT.conf_ch[pStrand->_rmtChannel].conf1.mem_rd_rst = 1;
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.tx_start = 1;

                    xSemaphoreTake(pStrand->_rmtSemaphore, portMAX_DELAY);
                    vSemaphoreDelete(pStrand->_rmtSemaphore);
                    pStrand->_rmtSemaphore = NULL;
                }
            }

            void start(void)
            {
                if (_started)
                    stop();

                DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN);
                DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST);

                RMT.apb_conf.fifo_mask = 1;      // Enable memory access, instead of FIFO mode
                RMT.apb_conf.mem_tx_wrap_en = 1; // Wrap around when hitting end of buffer

                for (int s = 0; s < MAX_STRANDS; s++)
                {
                    if (!_strands[s])
                        continue;
                    STRAND* pStrand = _strands[s];
                    neo_params_t ledParams = __neoParams[pStrand->_neoType];

                    rmt_set_pin(pStrand->_rmtChannel, RMT_MODE_TX, pStrand->_rmtPin);

                    RMT.conf_ch[pStrand->_rmtChannel].conf0.div_cnt = DIVIDER;
                    RMT.conf_ch[pStrand->_rmtChannel].conf0.mem_size = 1;
                    RMT.conf_ch[pStrand->_rmtChannel].conf0.carrier_en = 0;
                    RMT.conf_ch[pStrand->_rmtChannel].conf0.carrier_out_lv = 1;
                    RMT.conf_ch[pStrand->_rmtChannel].conf0.mem_pd = 0;

                    RMT.conf_ch[pStrand->_rmtChannel].conf1.rx_en = 0;
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.mem_owner = 0;
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.tx_conti_mode = 0; // loop back mode
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.ref_always_on = 1; // use apb clock: 80M
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.idle_out_en = 1;
                    RMT.conf_ch[pStrand->_rmtChannel].conf1.idle_out_lv = 0;

                    RMT.tx_lim_ch[pStrand->_rmtChannel].limit = MAX_PULSES;

                    // RMT config for transmitting a '0' bit val to this LED strand
                    pStrand->_rmtPulsePair[0].level0 = 1;
                    pStrand->_rmtPulsePair[0].level1 = 0;
                    pStrand->_rmtPulsePair[0].duration0 = ledParams.T0H / (RMT_DURATION_NS * DIVIDER);
                    pStrand->_rmtPulsePair[0].duration1 = ledParams.T0L / (RMT_DURATION_NS * DIVIDER);

                    // RMT config for transmitting a '0' bit val to this LED strand
                    pStrand->_rmtPulsePair[1].level0 = 1;
                    pStrand->_rmtPulsePair[1].level1 = 0;
                    pStrand->_rmtPulsePair[1].duration0 = ledParams.T1H / (RMT_DURATION_NS * DIVIDER);
                    pStrand->_rmtPulsePair[1].duration1 = ledParams.T1L / (RMT_DURATION_NS * DIVIDER);

                    RMT.int_ena.val |=
                        __tx_thr_event_offsets[pStrand->_rmtChannel];          // RMT.int_ena.ch<n>_tx_thr_event = 1;
                    RMT.int_ena.val |= __tx_end_offsets[pStrand->_rmtChannel]; // RMT.int_ena.ch<n>_tx_end = 1;

                    pStrand->clear();
                }

                esp_intr_alloc(ETS_RMT_INTR_SOURCE, 0, __rmtInterrupt, this, &this->_rmt_intr_handle);
                _started = true;

                render();
            }

            void stop(void)
            {
                if (!_started)
                    return;
                _started = false;
            }

        protected:
            void _initialise()
            {
                _started = false;
                _rmt_intr_handle = NULL;

                for (int s = 0; s < MAX_STRANDS; s++)
                {
                    if (_strands[s])
                        _strands[s]->_neoRender = this;
                }
            }

        private:
            DRIVER(DRIVER const& copy);            // Not Implemented
            DRIVER& operator=(DRIVER const& copy); // Not Implemented

            STRAND* _strands[MAX_STRANDS];
            bool _started;
            intr_handle_t _rmt_intr_handle;
        };

        // This fills half an RMT block
        // When wraparound is happening, we want to keep the inactive half of the RMT block filled
        //
        IRAM_ATTR void __rmtCopyBlock(STRAND* pStrand)
        {
            neo_params_t ledParams = __neoParams[pStrand->_neoType];
            uint16_t i, j, offset, len, byteval;

            offset = pStrand->_rmtHalf * MAX_PULSES;
            pStrand->_rmtHalf = !pStrand->_rmtHalf;

            len = pStrand->_pixelBytes - pStrand->_rmtIndex;

            if (len > (MAX_PULSES / 8))
                len = (MAX_PULSES / 8);

            if (!len)
            {
                if (!pStrand->_rmtDirty)
                {
                    return;
                }

                // Clear the channel's data block and return
                for (i = 0; i < MAX_PULSES; i++)
                {
                    RMTMEM.chan[pStrand->_rmtChannel].data32[i + offset].val = 0;
                }

                pStrand->_rmtDirty = pStrand->_isDirty = 0;
                return;
            }

            pStrand->_rmtDirty = 1;

            for (i = 0; i < len; i++)
            {
                byteval = pStrand->_pixelData[i + pStrand->_rmtIndex];

                // Shift bits out, MSB first, setting RMTMEM.chan[n].data32[x] to
                // the rmtPulsePair value corresponding to the buffered bit value
                for (j = 0; j < 8; j++, byteval <<= 1)
                {
                    int bitval = (byteval >> 7) & 0x01;
                    int data32_idx = i * 8 + offset + j;

                    RMTMEM.chan[pStrand->_rmtChannel].data32[data32_idx].val = pStrand->_rmtPulsePair[bitval].val;
                }

                // Handle the reset bit by stretching duration1 for the final bit in the stream
                if (i + pStrand->_rmtIndex == pStrand->_pixelBytes - 1)
                {
                    RMTMEM.chan[pStrand->_rmtChannel].data32[i * 8 + offset + 7].duration1 =
                        ledParams.TRS / (RMT_DURATION_NS * DIVIDER);
                }
            }

            // Clear the remainder of the channel's data not set above
            for (i *= 8; i < MAX_PULSES; i++)
            {
                RMTMEM.chan[pStrand->_rmtChannel].data32[i + offset].val = 0;
            }

            pStrand->_rmtIndex += len;
        }

        IRAM_ATTR void __rmtInterrupt(void* arg)
        {
            portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            DRIVER* pDriver = (DRIVER*)arg;

            for (int s = 0; s < MAX_STRANDS; s++)
            {
                STRAND* pStrand = pDriver->_strands[s];

                if (pStrand)
                {
                    if (RMT.int_st.val & __tx_thr_event_offsets[pStrand->_rmtChannel])
                    {
                        __rmtCopyBlock(pStrand);
                        RMT.int_clr.val |= __tx_thr_event_offsets[pStrand->_rmtChannel];
                    }
                    else if (RMT.int_st.val & __tx_end_offsets[pStrand->_rmtChannel] && pStrand->_rmtSemaphore)
                    {
                        xSemaphoreGiveFromISR(pStrand->_rmtSemaphore, &xHigherPriorityTaskWoken);
                        RMT.int_clr.val |= __tx_end_offsets[pStrand->_rmtChannel];

                        if (xHigherPriorityTaskWoken == pdTRUE)
                        {
                            portYIELD_FROM_ISR();
                        }
                    }
                }
            }
        }
    } // namespace NEO
} // namespace EZ
#endif // _EZ_HAL_NEO_H