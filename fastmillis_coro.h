/**************************************************************
 *  Make AceRoutine use fastmicros()
 **************************************************************/

#include <AceRoutine.h>

class FastClockInterface {
  public:
    /** Get the current millis. */
    static unsigned long millis() { return ::fastmillis(); }

    /** Get the current micros. */
    static unsigned long micros() { return ::fastmicros(); }

    static unsigned long cycles() { return ::xthal_get_ccount(); }

    /** This should be updated if cycles() is modified to count CPU cycles
     */
    static unsigned long cycles_per_second() { return 1000000*getCpuFrequencyMhz(); }

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
// using Coroutine = ace_routine::CoroutineTemplate<FastClockInterface>;
using Coroutine = ace_routine::CoroutineTemplate< ace_routine::Coroutine_Delay_32bit_Profiler_Impl< ace_routine::NamedCoroutine,FastClockInterface >>;
using CoroutineScheduler = ace_routine::CoroutineSchedulerTemplate<Coroutine>;
using Profiler = ace_routine::Profiler;


