/*
** EZIoT - Generic Millisecond Timer Class for all iot classes
**
** This program is provided free for you to use in any way that you wish,
** subject to the laws and regulations where you are using it.  Due diligence
** is strongly suggested before using this code.  Please give credit where due.
**
** The Author makes no warranty of any kind, express or implied, with regard
** to this program or the documentation contained in this document.  The
** Author shall not be liable in any event for incidental or consequential
** damages in connection with, or arising out of, the furnishing, performance
** or use of these programs.
*/
#ifndef _EZITIMER_H
#define _EZITIMER_H

#include <Arduino.h>

namespace EZ
{
    /*
    ** Timer Class
    */
    class TIMER
    {
    protected:
        unsigned long timerDuration;
        unsigned long startMillis;

    public:
        TIMER() {
            timerDuration = 0;
            startMillis = 0;
        }

        TIMER(unsigned long timer)
        {
            timerDuration = timer;
            startMillis = millis();
        }

        bool timerExpired(void)
        {
            if ((timerDuration == 0) || millis() - startMillis >= timerDuration)
                return true;
            return false;
        }

        inline void timerTrigger(void) { startMillis = millis() - timerDuration; }

        inline uint32_t timerPeriod(void) { return timerDuration; }

        inline void timerPeriod(unsigned long msPeriod) { timerDuration = msPeriod; }

        inline void timerReset(void) { startMillis = millis(); }
    };
} // namespace EZ
#endif // _EZITIMER_H
