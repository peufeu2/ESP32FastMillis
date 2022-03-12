# ESP32FastMillis
- Faster millis() and micros() for ESP32 and AceRoutine
- Improved OneWire library to make use of them
- some useful Chrono and Timeout classes, that can be used separately (just replace "fastmillis" with "millis").

Implementation of micros() on ESP32 is quite complicated, because it uses a 64 bit counter clocked on the CPU frequency, which requires a 64-bit division. In addition, the code checks for wraparounds etc. This means micros() takes non-constant time, around 2-3µs with 80MHz CPU frequency.

This wastes time (in my app, CoroutineScheduler was calling micros() about 15000 times per second).

Worse, this means OneWire does not work. OneWire relies on delayMicroseconds() for its bit-banging, and this calls micros(), which is too slow and not constant-time. OneWire thus reports a very high read error rate on the ESP32, about one in every 100 reads.

The fix is simple: ESP32 has timers with hardware predividers, so this small piece of code just sets up TIMG0_T0 to run on 1 MHz, dividing APB clock by 80. Then, micros() is just two instructions to read the 32-bit micros value from the timer. It is also possible to read a 64 bit microseconds value.

For milliseconds, TIMG0_T1 is configured to run at 2 kHz (because the 16-bit prescaler can divide 80MHz by 40000 but not 80000) and the code divides the value by two with a bti shift.

Due to the absence of division, there is no issue at wraparound. It's just a 64-bit counter.

The included arduino OneWire library has been modified to make use of this. It also uses an active pull-up, to drive long wires.

Included is some code to make AceRoutine use fastmillis() and fastmicros(), using the ClockInterface provided. If you don't use AceRoutine, just comment it out.



