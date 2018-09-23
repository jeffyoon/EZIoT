/*
** EZIoT
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
#if defined(ARDUINO_ARCH_ESP32)
#if !defined(_EZ_H)
#define _EZ_H
#include "core/iot.h"
#include "core/hal/hal_dmx.h"
#include "core/hal/hal_neo.h"

#endif // _EZ_H
#else  // ARDUINO_ARCH_ESP32
#error "EZIoT (currently) only supports ESP32 boards!"
#endif
/******************************************************************************/
