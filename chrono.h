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

/***************************************************************
 * 	Simple chrono class
 *	tick()	-> milliseconds elapsed since last call to tick()
 * 	total	-> milliseconds since reset(), updated by tick()
 ***************************************************************/
#include "fastmillis.h"

class Chrono {
public:
	uint32_t 	last_tick, 	// millis at last tick() call
				interval,	// "lap time"	ms between last two tick() calls
				total;		// "total time" ms between reset() and last tick()
	
	/*	First call to tick() will return 0ms and set initialized to true.
		This is to avoid having the first tick() return all the time spent
		in setup()...
	*/
	bool initialized = false;

	/*	Resets both total time and lap time, and starts the chrono
		by recording current time.
	*/
	void reset() { 
		initialized = true;
		last_tick = fastmillis(); 
		total = interval = 0; 
	}

	/*	Pushes the LAP button on the chronometer, and returns lap time
		ie, milliseconds since last calls to tick().
		If the value is needed again, just read member interval.
	 */
	uint32_t tick(){
		unsigned m = fastmillis();
		if( initialized ) interval = m-last_tick;
		initialized = true;
		last_tick = m;
		total += interval;
		return interval;
	}

	/*	Returns milliseconds since last call to tick().
	*/
	unsigned ms_since_tick() {
		return fastmillis() - last_tick;	
	}
};







