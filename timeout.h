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
 * 	Simple timeout class
 **************************************************************/

#include "fastmillis.h"

class Timeout {
public:
	uint32_t 	_end_millis;		// when the timeout should end
	bool 		_expired    = true;

	/*	To prevent 32 bit wraparound with millis(), one of the functions below
		should be called before 2^30 ms have passed. Once the timeout has expired,
		bool _expired is set to true, which remembers it expired for an unlimited
		amount of time.
	*/

	/*	Initializes timeout.
		expired() will return false until it expires, then it will return true.
	 */
	void set( unsigned long timeout_ms ) {
		if( timeout_ms ) {
			_end_millis = fastmillis() + timeout_ms;
			_expired = false;
		} else {
			_expired = true;
		}
	}

	/*	Mark it as expired now.
	*/
	void expire() {
		_expired = true;
	}


	/*	Returns time remaining in ms, zero if expired
	*/
	int32_t remaining() {
		if( _expired ) return 0;
		int r = _end_millis - fastmillis();
		if( r>0 )
			return r;
		_expired = true;	// prevent wraparound
		return 0;
	}


	/* true if timeout has expired
	*/
	bool expired() { return !remaining(); }

	/* 	Sets _expired to true if timeout has expired.
		Could just call remaining() but if we ignore the result,
		calling it tick() makes it more explicit. */
	void tick()  { remaining(); }	
};












