/*
 * DailyTimer Library
   Copyright(c) 2017, James Brower,  BulldogLowell@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#ifndef DailyTimer_h
#define DailyTimer_h

#include "Particle.h"

#define MAX_TIMER_INSTANCES 10

enum EventDays{SUNDAYS = 0, MONDAYS, TUESDAYS, WEDNESDAYS, THURSDAYS, FRIDAYS, SATURDAYS, WEEKENDS, WEEKDAYS, EVERY_DAY};
enum RandomType{FIXED, RANDOM, RANDOM_START, RANDOM_END};

const byte dayTemplate[] = {
  /*SMTWTFSS*/   // the bitmask is set with an extra bit for determining off times for days of the week where off time is earlier than on time (i.e. the two values stradle midnight)
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b10000010, // Weekends
  0b01111100, // Weekdays
  0b11111110  // Everyday
};

class DailyTimer{
  public:

    DailyTimer(byte StartHour, byte StartMinute, EventDays DaysOfTheWeek, RandomType type, void(*_startCallback)());
    DailyTimer(bool syncOnPowerup, byte StartHour, byte StartMinute, byte EndHour, byte EndMinute, EventDays DaysOfTheWeek, RandomType type, void(*_startCallback)(), void(*_endCallback)());

    void setDaysActive(EventDays days);
    void setDaysActive(byte activeDays);
    byte setRandomDays(byte number_Days);
    void setRandomOffset(uint8_t random_minutes, RandomType randomSetting);
    void setStartTime(uint8_t hour, uint8_t minute);
    void setEndTime(uint8_t hour, uint8_t minute);
    int getInstanceCount(void) const;
    bool begin();
    uint8_t getDays() const;
    static void update();
    void manualOn(void);
    void manualOff(void);
    void resume(void);

  protected:
    enum TimerState{
      TIMER_OFF,
      TIMER_ON,
    }timerState;
    struct TimerTime{ // bounded 00:00 to 23:59
      uint8_t hour;
      uint8_t minute;
    };
    bool sync();
    void(*startTimeCallback)(); //
    void(*endTimeCallback)();   //
    uint8_t onMask;                // compact ON days storage
    uint8_t offMask;               // compact OFF days storage
    bool state;                 // retains last true/false state of timer
    bool autoSync;              // will run startTimeCallback if timer is in active state when times are changed or powerup
    TimerTime startTime;        //
    TimerTime endTime;          //
    TimerTime randomStartTime;  // calculated once daily
    TimerTime randomEndTime;    // calculated once daily
    RandomType randomType;            //
    uint8_t currentDay;          // for comparison of a daily event to randomize the Start and end times
    uint8_t offset;             // minutes of fuzziness for random Starts and Ends

    static bool isActive(DailyTimer* instance);
    static time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);
    static DailyTimer* instanceAddress;
    static uint8_t instanceCount;
};

#endif

