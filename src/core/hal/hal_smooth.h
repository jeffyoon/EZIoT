/*
** EZIoT - Generic Signal Smoothing Class
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** Class based on code created 22 Apr 2007 by David A. Mellis  <dam@mellis.org>
** and later modified 9 Apr 2012 by Tom Igoe
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#ifndef _EZI_HAL_SMOOTH_H
#define _EZI_HAL_SMOOTH_H
#include <Arduino.h>

namespace EZ
{
    namespace HAL
    {
        template<class _T> class SMOOTH
        {
        public:
            SMOOTH(uint16_t numReadings = 10) : _nxtReading(0)
            {
                if (numReadings > 100)
                {
                    numReadings = 100;
                }
                if (numReadings < 1)
                {
                    numReadings = 1;
                }
                _numReadings = numReadings;

                _rawReadings = new _T[_numReadings];
                memset(_rawReadings, 0, sizeof(_T) * _numReadings);
            }

            ~SMOOTH() { delete _rawReadings; }

            _T smooth(_T value)
            {
                if (_numReadings <= 1)
                    return value;
                _rawReadings[_nxtReading++] = value;

                _T total = 0;
                for (int r = 0; r < _nxtReading; r++)
                    total += _rawReadings[r];
                total /= _nxtReading > 0 ? _nxtReading : 1;

                if (_nxtReading >= _numReadings)
                    _nxtReading = 0;

                return total;
            }

        protected:
            uint16_t _nxtReading;
            uint16_t _numReadings;
            _T* _rawReadings;
        };
    } // namespace HAL
} // namespace EZ
#endif // _EZI_HAL_SMOOTH_H
/******************************************************************************/
