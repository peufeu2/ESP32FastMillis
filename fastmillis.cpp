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

#include "config.h"
#include "fastmillis.h"

/**************************************************************
 *	ESP32
 * 	Much faster millis() / micros() implementation
 * 	using hardware timer to avoid the mess that is
 *  esp_timer_get_time()
 **************************************************************/

void init_TIMG0() {
	timerBegin(0, 80, true);
	timerBegin(1, 40000, true);
	return;

	// divide APB_CLK by 80 to get 1MHz, for 1µs timer
	unsigned divider = 80;
	TIMG0_T0CONFIG_REG = 0x00000000 | (divider<<13);	// timer must be disabled to change prescaler setting
	TIMG0_T0CONFIG_REG = 0xC0000000 | (divider<<13);

	// divide APB_CLK by 40000 to get 2kHz, for 0.5ms timer (max divider is 65536)
	divider = 40000;
	TIMG0_T1CONFIG_REG = 0x00000000 | (divider<<13);	// timer must be disabled to change prescaler setting
	TIMG0_T1CONFIG_REG = 0xC0000000 | (divider<<13);
}

void IRAM_ATTR fastDelayMicroseconds(uint32_t us)
{
    if( !us ) return;
    uint32_t m = fastmicros();
    uint32_t e = (m + us);
    if(m > e){ //overflow
        while(fastmicros() > e){
            NOP();
        }
    }
    while(fastmicros() < e){
        NOP();
    }
}

void fastDelayMicroseconds_withInterrupts( uint32_t us ) {
    if( !us ) return;
    uint32_t m = fastmicros();

    interrupts();

    /*  Wait with interrupts enabled.
        We stop 50µs before the end.
    */

    uint32_t e = (m + us);
    while( int32_t(e - fastmicros()) > 50 )
        NOP();

    noInterrupts();

    /*  As we get near the end of the delay,
        enable interrupts again to get a more
        accurate end time.
    */

    while( int32_t(e - fastmicros()) >= 0 )
        NOP();
}
