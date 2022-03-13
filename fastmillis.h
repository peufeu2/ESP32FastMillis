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
#define TIMG0_T0LOAD_LO_REG (*(volatile uint32_t*)(0x3FF5F018)) 
#define TIMG0_T0LOAD_HI_REG (*(volatile uint32_t*)(0x3FF5F01C)) 
#define TIMG0_T0LOAD_REG    (*(volatile uint32_t*)(0x3FF5F020)) 

#define TIMG0_T1CONFIG_REG (*(volatile unsigned *)(0x3FF5F024)) // configuration register
#define TIMG0_T1LO_REG     (*(volatile unsigned *)(0x3FF5F028))
#define TIMG0_T1HI_REG     (*(volatile unsigned *)(0x3FF5F02C))
#define TIMG0_T1UPDATE_REG (*(volatile unsigned *)(0x3FF5F030))
#define TIMG0_T1LOAD_LO_REG (*(volatile uint32_t*)(0x3FF5F03C)) 
#define TIMG0_T1LOAD_HI_REG (*(volatile uint32_t*)(0x3FF5F040)) 
#define TIMG0_T1LOAD_REG    (*(volatile uint32_t*)(0x3FF5F044)) 

/*  INTERRUPT SAFE, usable in interrupts and userland code
    This does only one load, so it doesn't have any wraparound issues.
    If an interrupt hits between the two lines of code below, and writes to
    TIMG0_T0UPDATE_REG, the worst that can happen is that we get a more up-to-date
    microsecond counter, because it was updated in the ISR.
*/
static inline uint32_t IRAM_ATTR fastmicros() {
  TIMG0_T0UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  return TIMG0_T0LO_REG;  // read registers
}

/*  INTERRUPT SAFE, usable in interrupts and userland code
    This loads the counter value twice to make sure fastmillis() called from an
    ISR didn't mess with it between the two loads. 
*/
uint32_t fastmillis();

/*  INTERRUPT SAFE, usable in interrupts and userland code
    This loads the counter value twice to make sure fastmillis() called from an
    ISR didn't mess with it between the two loads. 
*/
uint64_t fastmicros64();

/*  NOT INTERRUPT SAFE
    /!\ WARNING: If an interrupt occurs between the two loads, and the ISR writes to 
    TIMG0_T0UPDATE_REG, then the second 32-bit value read will be wrong, which means the
    entire 64-bit value read will be wrong.

    If you don't use any of fastmillis/fastmicros() in your interrupts, then
    there is no problem.

    Otherwise only call this in noInterrupts() block.
*/
static inline uint64_t IRAM_ATTR fastmicros64_isr() {
  TIMG0_T0UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  return (uint64_t(TIMG0_T0HI_REG)<<32) | TIMG0_T0LO_REG;
}


/*  fastdelayMicroseconds isn't faster (it's a delay!) but it is much more accurate.
    ESP32 delayMicroseconds() polls micros() which takes a NON-CONSTANT TIME of about 
    2-3µs to run at 80MHz CPU clock. 

    This one polls fastmicros() which is only 2 instructions, thus constant time.

    Modifying OneWire.cpp to use this, reduces number of read errors substantially.
*/
void fastDelayMicroseconds( uint32_t us );

/*  For longer but still somewhat accurate delays, we want to keep interrupts enabled.
    This function expects interrupts to be DISABLED before it is called.
    It will enable interrupts during the wait, then DISABLE THEM AGAIN
    before returning.
*/
void fastDelayMicroseconds_withInterrupts( uint32_t us );

/**************************************************************
 *  Make AceRoutine use fastmicros()
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
