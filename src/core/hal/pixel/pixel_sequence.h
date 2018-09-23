/*
** EZIoT - Pixel Seqeunces
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
#ifndef _EZ_PIXEL_SEQUENCE_H
#define _EZ_PIXEL_SEQUENCE_H
#include "core/tool/ez_color.h"
#include <Arduino.h>

namespace EZ
{
    namespace PIXEL
    {
        typedef struct PATTERN
        {
            struct type
            {
                uint8_t timer : 3, onFrame : 1, onCycle : 1;
            };

            uint8_t mode;
            uint8_t level;
            uint8_t speed;
            uint8_t flags;
            COLOR color1;
            COLOR color2;
            uint32_t count;

            // Constructors
            //
            PATTERN();

        } pattern_t;

    } // namespace PIXEL
} // namespace EZ
#endif // _EZ_PIXEL_SEQUENCE_H