#include "config.h"
#include "fastmillis.h"

/**************************************************************
 *	ESP32
 * 	Much faster millis() / micros() implementation
 * 	using hardware timer to avoid the mess that is
 *  esp_timer_get_time()
 **************************************************************/

void init_TIMG0() {

	// divide APB_CLK by 80 to get 1MHz, for 1µs timer
	unsigned divider = 80;
	TIMG0_T0CONFIG_REG = 0x00000000 | (divider<<13);	// timer must be disabled to change prescaler setting
	TIMG0_T0CONFIG_REG = 0xC0000000 | (divider<<13);

	// divide APB_CLK by 40000 to get 2kHz, for 0.5ms timer (max divider is 65536)
	divider = 40000;
	TIMG0_T1CONFIG_REG = 0x00000000 | (divider<<13);	// timer must be disabled to change prescaler setting
	TIMG0_T1CONFIG_REG = 0xC0000000 | (divider<<13);
}

