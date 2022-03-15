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

    TIMG0_T0LOAD_LO_REG = 0xFF000000;
    TIMG0_T0LOAD_HI_REG = 0;
    TIMG0_T0LOAD_REG = 1;

	// divide APB_CLK by 40000 to get 2kHz, for 0.5ms timer (max divider is 65536)
	divider = 40000;
	TIMG0_T1CONFIG_REG = 0x00000000 | (divider<<13);	// timer must be disabled to change prescaler setting
	TIMG0_T1CONFIG_REG = 0xC0000000 | (divider<<13);

    TIMG0_T1LOAD_LO_REG = 0xFF000000;
    TIMG0_T1LOAD_HI_REG = 0;
    TIMG0_T1LOAD_REG = 1;
}

uint32_t IRAM_ATTR fastmillis() {
  TIMG0_T1UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  uint32_t lo, lo2, hi;
  do {
    lo = TIMG0_T1LO_REG;
    hi = TIMG0_T1HI_REG;
    lo2 = TIMG0_T1LO_REG;
  } while( lo != lo2 );
  return (uint64_t(hi)<<32) | (lo>>1); // we divided by 40000, now divide by 2 to get milliseconds
}

uint64_t IRAM_ATTR fastmicros64() {
  TIMG0_T0UPDATE_REG = 0; // write here to tell the hardware to copy counter value into read registers
  uint32_t lo, lo2, hi;
  do {
    lo = TIMG0_T0LO_REG;
    hi = TIMG0_T0HI_REG;
    lo2 = TIMG0_T0LO_REG;
  } while( lo != lo2 );
  return (uint64_t(hi)<<32) | lo;
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

void IRAM_ATTR accurateDelayMicroseconds(uint32_t us)
{
    if( !us ) return;
    uint32_t m = xthal_get_ccount();
    uint32_t e = m + us*CPU_FREQUENCY_MHZ - 44;     // substract some cycles to account for function call
    if(m > e){ //overflow
        while(xthal_get_ccount() > e){
            NOP();
        }
    }
    while(xthal_get_ccount() < e){
        NOP();
    }
}
