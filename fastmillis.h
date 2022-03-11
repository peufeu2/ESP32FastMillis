/*
MIT License

Copyright (c) 2022 peufeu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <AceRoutine.h>

/**************************************************************
 * 	Much faster millis() / micros() implementation
 * 	using hardware timer to avoid the mess that is
 *  esp_timer_get_time()
 **************************************************************/

void init_TIMG0();

#define TIMG0_T0CONFIG_REG (*(volatile unsigned *)(0x3FF5F000)) // configuration register
#define TIMG0_T0LO_REG     (*(volatile uint32_t*)(0x3FF5F004)) // bottom 32-bits of the timer value
#define TIMG0_T0HI_REG     (*(volatile uint32_t*)(0x3FF5F008)) // top 32-bits of the timer value
#define TIMG0_T0UPDATE_REG (*(volatile uint32_t*)(0x3FF5F00C)) // write any value this to latch the timer value into hi_reg and lo_reg

#define TIMG0_T1CONFIG_REG (*(volatile unsigned *)(0x3FF5F024)) // configuration register
#define TIMG0_T1LO_REG     (*(volatile unsigned *)(0x3FF5F028))
#define TIMG0_T1HI_REG     (*(volatile unsigned *)(0x3FF5F02C))
#define TIMG0_T1UPDATE_REG (*(volatile unsigned *)(0x3FF5F030))

static inline uint32_t fastmicros() {
  TIMG0_T0UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  return TIMG0_T0LO_REG;  // read registers
}

static inline uint64_t fastmicros64() {
  TIMG0_T0UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  return (TIMG0_T1HI_REG<<32) | TIMG0_T1LO_REG; // due to above feature, we can read the two registers
                                                // without them changing during the read.
}

static inline uint32_t fastmillis() {
  TIMG0_T1UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  uint64_t t = (TIMG0_T1HI_REG<<32) | TIMG0_T1LO_REG;
  return t>>1;          // we divided by 40000, now divide by 2 to get milliseconds
}


/**************************************************************
 *  Make AceRoutine use fastmicros()
 *  This bit copypasted from AceRoutine
 *  https://github.com/bxparks/AceRoutine/blob/develop/src/ace_routine/ClockInterface.h
 **************************************************************/

class FastClockInterface {
  public:
    /** Get the current millis. */
    static unsigned long millis() { return ::fastmillis(); }

    /** Get the current micros. */
    static unsigned long micros() { return ::fastmicros(); }

    /**
     * Get the current seconds. This is derived by dividing millis() by 1000,
     * which works pretty well until the `unsigned long` rolls over at
     * 4294967296 milliseconds. At that last second (4294967), this function
     * returns the next second (0) a little bit too early. More precisely, it
     * rolls over to 0 seconds 704 milliseconds too early. If the
     * COROUTINE_DELAY_SECONDS() is large enough, this inaccuracy should not
     * matter too much.
     *
     * The other problem with this function is that on 8-bit processors without
     * a hardware division instruction, the software long division by 1000 is
     * very expensive in both memory and CPU. The flash memory increases by
     * about 150 bytes on AVR processors. Therefore, the
     * COROUTINE_DELAY_SECONDS() should be used sparingly.
     */
    static unsigned long seconds() { return ::fastmillis() / 1000; }
};

// using namespace ace_routine;

// Use fastmicros() for aceroutine
using Coroutine = ace_routine::CoroutineTemplate<FastClockInterface>;
using CoroutineScheduler = ace_routine::CoroutineSchedulerTemplate<Coroutine>;
