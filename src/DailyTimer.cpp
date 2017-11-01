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

   Library provides tools to set daily timers for control of devices such as lamps, appliances, etc. Developed primarilary for houshold presence simulation.
   Allows for setting ON and OFF times, days of the week (i.e. weekends, weekdays, Fridays) and the use of random on/off times
   Timers may bridge midnight, simply enter times accordingly:
   Automatically sets correct timer state on powerup, see example below.
   Randomize start and/or end times using this member function:
   Random days of week using setRandomDays(4); // will select four days of the week, randomly.  Useful to run this member function once a week, for example.
   Select custom days of the week using setDaysActive(0b10101010);  // e.g.Sunday, Tuesday, Thursday and Saturday Note: Days in this order:  0bSMTWTFS0 <- LSB is zero
   Set a timed event with just the start time as a trigger:
   you can return the active days using getDays();  // returns a byte... Sunday is the MSB, Saturday is the LSB << 1
   Dynamically set your start or end time (i.e. using some type of celestial function or web call to determine Sunrise or Sunset times)
   myTimer.setStartTime(byte hour, byte minute);
   myTimer.setEndTime(byte hour, byte minute);

*/

#include "DailyTimer.h"

uint8_t DailyTimer::instanceCount = 0;
DailyTimer* instances[MAX_TIMER_INSTANCES] = {nullptr};


DailyTimer::DailyTimer(byte StartHour, byte StartMinute, EventDays DaysOfTheWeek, RandomType type, void(*StartTimeCallback)())
{
  autoSync = false;
  startTime.hour = StartHour > 23 ? 23 : StartHour;
  startTime.minute = StartMinute > 59 ? 59 : StartMinute;
  endTime.hour = startTime.hour;
  endTime.minute = startTime.minute;
  setDaysActive(dayTemplate[static_cast<int>(DaysOfTheWeek)]);
  randomType = type;
  offset = 15;
  startTimeCallback = StartTimeCallback;
  endTimeCallback = NULL;
  instances[instanceCount] = this;
  instanceCount++;
  timerState = TIMER_ON;
}

DailyTimer::DailyTimer(bool syncOnPowerup, byte StartHour, byte StartMinute, byte EndHour, byte EndMinute, EventDays DaysOfTheWeek, RandomType type, void(*StartTimeCallback)(), void(*EndTimeCallback)())
{
  autoSync = syncOnPowerup;
  startTime.hour = StartHour > 23 ? 23 : StartHour;
  startTime.minute = StartMinute > 59 ? 59 : StartMinute;
  endTime.hour = EndHour > 23 ? 23 : EndHour;
  endTime.minute = EndMinute > 59 ? 59 : EndMinute;
  setDaysActive(dayTemplate[static_cast<int>(DaysOfTheWeek)]);
  randomType = type;
  offset = 15;
  startTimeCallback = StartTimeCallback;
  endTimeCallback = EndTimeCallback;
  instances[instanceCount++] = this;
  timerState = TIMER_ON;
}

int DailyTimer::getInstanceCount(void) const
{
  return instanceCount;
}

bool DailyTimer::begin()
{
  return sync();
}

void DailyTimer::setDaysActive(EventDays days)
{
  onMask = dayTemplate[static_cast<int>(days)];
  time_t now_time = Time.local();
  if(tmConvert_t(Time.year(), Time.month(), Time.day(), startTime.hour, startTime.minute, 0) > tmConvert_t(Time.year(), Time.month(), Time.day(), endTime.hour, endTime.minute, 0))
  {
    offMask = onMask >> 1;
  }
  else
  {
    offMask = onMask;
  }
  (void)sync();
}

void DailyTimer::setDaysActive(byte activeDays)
{
  onMask = activeDays;
  time_t now_time = Time.local();
  if(tmConvert_t(Time.year(), Time.month(), Time.day(), startTime.hour, startTime.minute, 0) > tmConvert_t(Time.year(), Time.month(), Time.day(), endTime.hour, endTime.minute, 0))
  {
    offMask = onMask >> 1;
  }
  else
  {
    offMask = onMask;
  }
  (void)sync();
}

void DailyTimer::setRandomOffset(uint8_t random_minutes, RandomType type)
{
  offset = random_minutes > 59 ? 59 : random_minutes;
  if (offset == 0)
  {
    randomType = FIXED;
  }
  else
  {
    randomType = type;
  }
}

void DailyTimer::setStartTime(uint8_t hour, uint8_t minute)
{
  startTime.hour = hour > 23 ? 23 : hour;
  startTime.minute = minute > 59 ? 59 : minute;
  setDaysActive(onMask);
  (void)sync();
}

void DailyTimer::setEndTime(uint8_t hour, uint8_t minute)
{
  endTime.hour = hour > 23 ? 23 : hour;
  endTime.minute = minute > 59 ? 59 : minute;
  setDaysActive(onMask);
  (void)sync();
}


uint8_t DailyTimer::setRandomDays(uint8_t number_Days)
{
  randomSeed(Time.now() + micros());  // not local time on purpose
  uint8_t mask = 0;
  uint8_t array[8] = {0};
  for (int i = 0; i < number_Days; i++)
  {
    array[i] = 1;
  }
  for(int i = 0; i < 7; i++)
  {
    uint8_t index = random(i, 7);
    uint8_t temp = array[i];
    array[i] = array[index];
    array[index] = temp;
  }
  for (int i = 0; i < 7; i++)
  {
    mask |= (array[i] << i);
  }
  onMask = mask << 1;
  (void)sync();
  return onMask;
}

byte DailyTimer::getDays() const
{
  return onMask;
}

bool DailyTimer::sync()
{
  bool currentState = isActive(this);
  if(currentState && autoSync)
  {
    if(startTimeCallback)
      startTimeCallback();
  }
  else if(!currentState && autoSync)
  {
    if (endTimeCallback)
      endTimeCallback();
  }
  return state = currentState;
}

void DailyTimer::update()
{
  for(int i = 0; i < instanceCount; i++)
  {
    bool lastState = instances[i]->state;
    instances[i]->state = isActive(instances[i]);
    if(lastState != instances[i]->state)
    {
      if(instances[i]->state == true)
      {
        if(instances[i]->startTimeCallback) instances[i]->startTimeCallback();
      }
      else
      {
        if(instances[i]->endTimeCallback) instances[i]->endTimeCallback();
      }
    }
  }
}

bool DailyTimer::isActive(DailyTimer* instance)
{
  if (instance->timerState == TIMER_OFF)
    return false;
  if (instance->currentDay != Time.weekday() && instance->randomType) // once a day, generate new random offsets
  {
    randomSeed(Time.now() + micros());  // not local time on purpose
    if (instance->randomType == RANDOM  || instance->randomType == RANDOM_START)
    {
      int hrs = instance->startTime.hour * 60 + instance->startTime.minute;
      hrs += constrain(random(-1 * instance->offset, instance->offset), 1, (24 * 60) - 1);
      instance->randomStartTime.minute = hrs % 60;
      instance->randomStartTime.hour = hrs / 60;
    }
    if (instance->randomType == RANDOM || instance->randomType == RANDOM_END)
    {
      int hrs = instance->endTime.hour * 60 + instance->endTime.minute;
      hrs += constrain(random(-1 * instance->offset, instance->offset), 1, (24 * 60) - 1);
      instance->randomEndTime.minute = hrs % 60;
      instance->randomEndTime.hour = hrs / 60;
    }
    instance->currentDay = Time.weekday();
  }
  time_t now_time = Time.local();
  time_t on_time = tmConvert_t(Time.year(), Time.month(), Time.day(), (instance->randomType == RANDOM || instance->randomType == RANDOM_START) ? instance->randomStartTime.hour : instance->startTime.hour, (instance->randomType == RANDOM || instance->randomType == RANDOM_START) ? instance->randomStartTime.minute : instance->startTime.minute, /*second(now_time)*/ 0);
  time_t off_time = tmConvert_t(Time.year(), Time.month(), Time.day(), (instance->randomType == RANDOM || instance->randomType == RANDOM_END) ? instance->randomEndTime.hour : instance->endTime.hour, (instance->randomType == RANDOM || instance->randomType == RANDOM_END) ? instance->randomEndTime.minute : instance->endTime.minute, /*second(now_time));*/ 0);
  byte weekDay = Time.weekday(now_time);
  byte today = 0b00000001 << (8 - weekDay);
  if (today & dayTemplate[SUNDAYS])
  {
    today |= 0b00000001;
  }
  if ((today & instance->onMask) && (today & instance->offMask))  // if it is supposed to turn both on and off today
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else if (off_time < on_time)
    {
      return (now_time > on_time || now_time < off_time);
    }
    else if(on_time == off_time) // single edge event
    {
      if( now_time == on_time)
      {
        return true;
      }
    }
    return false;
  }
  else if (today & instance->onMask) // if it is supposed to turn only on today
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else
    {
      return (now_time > on_time);
    }
  }
  else if (today & instance->offMask)  // if it is supposed to turn only off today
  {
    return now_time < off_time;
  }
  else // if
  {
    return false;
  }
}

time_t DailyTimer::tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  struct tm t;
  t.tm_year = YYYY-1900;
  t.tm_mon = MM - 1;
  t.tm_mday = DD;
  t.tm_hour = hh;
  t.tm_min = mm;
  t.tm_sec = ss;
  t.tm_isdst = 0;
  time_t t_of_day = mktime(&t);
  return t_of_day;
}

void DailyTimer::manualOn(void)
{
  timerState = TIMER_OFF;
  startTimeCallback();
}

void DailyTimer::manualOff(void)
{
  timerState = TIMER_OFF;
  endTimeCallback();
}

void DailyTimer::resume(void)
{
  timerState = TIMER_ON;
  begin();
}
