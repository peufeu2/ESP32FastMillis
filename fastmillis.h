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

static inline uint32_t fastmillis() {
  TIMG0_T1UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  uint64_t t = (TIMG0_T1HI_REG<<32) | TIMG0_T1LO_REG;
  return t>>1;          // we divided by 40000, now divide by 2 to get milliseconds
}


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
