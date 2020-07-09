/* timekeeper.cpp - maintain the time and date
 *
 * Part of dcfclock
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
*/
#include <timekeeper.h>
#include <displaydriver.h>

#define TICKS_PER_SECOND	1000	// Using the Arduino "millis" timebase
// #define TICKS_PER_SECOND	50		// Using a 50 Hz signal derived from the mains

// Current time and date in local time
unsigned days;			// No. of days since 2020-01-01. 16-bit gives over 175 years
unsigned char hours;	// No. of hours  0..23
unsigned char mins;		// No. of minutes 0..59
unsigned char secs;		// No of seconds 0..59

void TimekeeperInit(task_t *timekeeperTask)
{
	timekeeperTask->timer = TICKS_PER_SECOND;
}

void Timekeeper(task_t *timekeeperTask, unsigned long elapsed)
{
	timekeeperTask->timer += TICKS_PER_SECOND;

	secs++;
	if ( secs >= 60 )
	{
		secs = 0;
		mins++;
		if ( mins >= 60 )
		{
			mins = 0;
			hours++;
			if ( hours >= 24 )
			{
				hours = 0;
				days++;

				// ToDo: DDMM and YYYY display modes
			}
		}

		// If mode is hh:mm, update the display when the minutes change
		if ( display_mode == mode_hhmm )
		{
			setdigitnumeric(3, mins % 10);
			setdigitnumeric(2, mins / 10);
			setdigitnumeric(1, hours % 10);
			if ( hours < 10 )
				setdigitsegments(0, 0);			// Blank when hours < 10
			else
				setdigitnumeric(0, hours / 10);
			display_change |= change_digits;
		}
	}

	// If mode is mm:ss, update the display when the seconds change.
	if ( display_mode == mode_mmss )
	{
		setdigitnumeric(3, secs % 10); 
		setdigitnumeric(2, secs / 10);
		setdigitnumeric(1, mins % 10);
		setdigitnumeric(0, mins / 10);
		display_change |= change_digits;
	}

	// If mode is any time mode, flash the colon
	if ( display_mode == mode_mmss || display_mode == mode_hhmm )
	{
		setcolon(secs & 0x01);						// Flashing colon for time display
		display_change |= change_leds;
	}
}
