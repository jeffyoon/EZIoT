/*
** EZIoT - DMX Lights
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
#ifndef _EZ_HAL_DMX_H
#define _EZ_HAL_DMX_H
#include "core/tool/ez_color.h"
#include "pixel/pixel_handler.h"
#include "pixel/pixel_segment.h"

namespace EZ
{   
    namespace DMX
    {
        class UNIVERSE : public PIXEL::HANDLER
        {
            public:
            protected:
            private:
        };
    } // namespace DMX
} // namespace EZ
#endif // _EZ_HAL_DMX_H