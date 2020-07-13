/* dcfclock.h - a DFC-77 clock using an Arduino Nano
 *
 * (c) David Haworth
 *
 * dcfclock is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dcfclock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dcfclock.  If not, see <http://www.gnu.org/licenses/>.
 *
 * dcfclock is an Arduino sketch, written for an Arduino Nano
 *
 *
*/
#ifndef DCFCLOCK_H
#define DCFCLOCK_H		1

#include <Arduino.h>

#define Time_millis		0
#define Time_50Hz		1		// Mains frequency
#define Time_100Hz		2		// Mains frequency (full-wave rectified)

#define TimeSource		Time_millis

#if TimeSource == Time_millis
#define Ticks(x)	((x))
#elif TimeSource == Time_50Hz
#define Ticks(x)	((x)/20)
#elif TimeSource == Time_100Hz
#define Ticks(x)	((x)/10)
#endif

extern unsigned ReadTime(void);

#endif
