/*
DailyTimer.h Library for Particle Devices
BulldogLowell@gmail.com
April, 2016
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

#include "DailyTimer.h"

void ledOn(void);  // function definitions/prototypes required here
void ledOff(void);
void printSomething(void);

const byte ledPin = D7;

DailyTimer ledTimer1(
  true,                             // AutoSync true or false, will run the startTimeCallback() if restarts within the active range or after range changes and you are in the range
  10,                               // Start Hour
  54,                               // Start Minute
  10,                                // End Hour
  57,                                // End Minute
  MONDAYS,                         // SUNDAYS, MONDAYS, TUESDAYS, WEDNESDAYS, THURSDAYS, FRIDAYS, SATURDAYS, WEEKENDS, WEEKDAYS, or EVERY_DAY
  FIXED,                            // OPTIONAL - FIXED, RANDOM, RANDOM_START, or RANDOM_END
  ledOn,                            // pointer to function to execute at Start time, or a Lambda as in this example:
//  []{digitalWrite(ledPin, HIGH);},  // Lambda equivalent example rather than creating a new function -> no callback function prototypes needed :)
  ledOff                            // pointer to function to execute at End time
);

DailyTimer serialPrintTimer(
  10,
  55,
  EVERY_DAY,
  FIXED,
  //printSomething
  []{Serial.println("Serial Print Timer just Fired!");}
);

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  Time.zone(-4);
  Serial.print("Active Timers=");
  Serial.println(ledTimer1.getInstanceCount());

//  example usage:
//  ledTimer1.setRandomOffset(10, RANDOM); // changes both the start time and end time to Random, +- 10 minutes
//  ledTimer1.setRandomOffset(10, RANDOM_START); // changes both the start time to RANDOM_START, +- 10 minutes
//  ledTimer1.setRandomOffset(10, RANDOM_END); // changes both the end time to RANDOM_END, +- 10 minutes
//  ledTimer1.setRandomOffset(10, FIXED); // changes both the start time and end time to FIXED

//  ledTimer1.setDaysActive(WEEKENDS);  // changes to SUNDAYS, MONDAYS, TUESDAYS, WEDNESDAYS, THURSDAYS, FRIDAYS, SATURDAYS, WEEKENDS, WEEKDAYS, or EVERY_DAY
//  ledTimer1.setDaysActive(0b11100000); // SUNDAYS, MONDAYS and TUESDAYS in this example

//  ledTimer1.setRandomDays(3);  // chooses three random days of the week

//  ledTimer1.setStartTime(11, 30); // set the start time
//  ledTimer1.setEndTime(23, 59);   // and the end time

  ledTimer1.begin();  // syncs the timer, use it here and after calls to setStartTime
  Serial.print("LedTimer Active days: ");
  Serial.println(ledTimer1.getDays(), BIN);
  serialPrintTimer.begin();
  Serial.print("Serial Print Active days: ");
  Serial.println(serialPrintTimer.getDays(), BIN);
}

void loop()
{
  static int lastSecond = 60;
  DailyTimer::update();
  if(Time.second() != lastSecond)
  {
    char timeBuffer[32] = "";
    sprintf(timeBuffer, "Time:%2d:%02d:%02d\tDate:%02d/%02d/%4d", Time.hour(), Time.minute(), Time.second(), Time.month(), Time.day(), Time.year());
    Serial.println(timeBuffer);
    lastSecond = Time.second();
  }
}

void ledOn(void)
{
  Serial.println("LED ON FUNCTION");
  digitalWrite(ledPin, HIGH);
}

void ledOff(void)
{
  Serial.println("LED OFF FUNCTION");
  digitalWrite(ledPin, LOW);
}

void printSomething(void)
{
  Serial.println("Something");
}
