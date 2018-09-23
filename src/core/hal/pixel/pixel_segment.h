/*
** EZIoT - Pixel Segment Class
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
#ifndef _EZ_PIXEL_SEGMENT_H
#define _EZ_PIXEL_SEGMENT_H
#include "pixel_handler.h"

namespace EZ
{
    namespace PIXEL
    {
        class SEGMENT : public HANDLER
        {
        public:
            SEGMENT(PIXEL::HANDLER& pxb) : SEGMENT(pxb, 0, pxb.pixels()) {}
            SEGMENT(PIXEL::HANDLER& pxb, uint16_t offset, uint16_t count)
                : HANDLER(pxb._pixelOrder, 0), _pixelBuffer(&pxb), _pixelOffset(offset)
            {
                if ((_pixelOffset = offset) < pxb.pixels() && offset + count <= pxb.pixels())
                {
                    if (count)
                    {
                        __pixelOrder(pxb._pixelOrder);
                        __pixelCount(count);
                        clear();
                    }
                }
                else
                    _pixelOffset = _pixelCount = 0;
            }

            ~SEGMENT() {}

        protected:
            HANDLER* _pixelBuffer;
            uint16_t _pixelOffset;

            void _setDirty(void) final
            {
                if (_pixelBuffer)
                    _pixelBuffer->_isDirty = true;
            }

            uint8_t* _memAlloc(size_t size)
            {
                return _pixelBuffer ? &_pixelBuffer->_pixelData[_pixelSize * _pixelOffset] : nullptr;
            }

            uint8_t* _memFree(uint8_t* mem) { return nullptr; }

        private:
            SEGMENT(SEGMENT const& copy);            // Not Implemented
            SEGMENT& operator=(SEGMENT const& copy); // Not Implemented
        };
    } // namespace PIXEL
} // namespace EZ
#endif // _EZ_PIXEL_SEGMENT_H