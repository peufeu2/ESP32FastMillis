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

/**************************************************************
 *  Implementation of interrupt disable
 **************************************************************/

/* interrupts() / noInterrupts() does not disable interrupts */

/* does not disable interrupts */
// #define timeCriticalEnter() do { static unsigned   esp_int_level = portSET_INTERRUPT_MASK_FROM_ISR();
// #define timeCriticalExit()   portCLEAR_INTERRUPT_MASK_FROM_ISR(esp_int_level); } while(0)

/* this works */
#undef noInterrupts()
#undef interrupts()

#define timeCriticalEnter() {portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;portENTER_CRITICAL(&mux);
#define timeCriticalExit() portEXIT_CRITICAL(&mux);}


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

    Note accuracy is 1µs due to using a 1MHz timer. If the function begins when the
    timer is about to count up, then the first microsecond will be truncated.

*/
void fastDelayMicroseconds( uint32_t us );


/*  This one polls xthal_get_ccount() which is a CPU register counting clock cycles,
    so it will be the most accurate of all, but it depends on #define CPU_FREQUENCY_MHZ 
    to be there and the clock frequency to actually be set to that value.

    It is only usable (and should only be used) for short delays, since the number of 
    CPU cycles has to fit into 31 bits.
*/
void accurateDelayMicroseconds( uint32_t us );

/*  
    When bit-banging signals...

    output(1)
    delayMicroseconds( period );
    output(0)
    delayMicroseconds( period );
    output(1)
    etc...

    When we get to the third delay, errors on each individual delay add up,
    plus the time taken to execute the instructions in-between.

    This class takes a snapshot of the cycle counter, and all calls to waitUntil()
    refer to that. So the above would become:

    MultiDelay d;
    output(1)
    d.waitUntilMicros( period );
    output(0)
    d.waitUntilMicros( period*2 );
    output(1)
    etc...

    And errors will not accumulate.

*/
class MultiDelay {
public:
    uint32_t start_cycles;

    inline __attribute__((always_inline)) 
    void reset() {
        start_cycles = xthal_get_ccount(); 
    }

    inline __attribute__((always_inline)) 
    MultiDelay() { 
        reset(); 
    }

    /*  Waits until we're "us" later than when reset() was called.
    */
    inline __attribute__((always_inline)) 
    void waitUntilMicros( int us ) { 
        waitUntilCycles( us*CPU_FREQUENCY_MHZ );
    }

    /*  Waits until we're "cycles" cpu cycles later than when reset() was called.
    */
    inline __attribute__((always_inline)) 
    void waitUntilCycles( int cycles ) { 
        while( (int)(xthal_get_ccount() - start_cycles) < cycles ) ;
    }

    inline __attribute__((always_inline)) 
    int elapsedCycles() {
        return xthal_get_ccount() - start_cycles; 
    }
};





