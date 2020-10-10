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
#include "dcfclock.h"
#include "timekeeper.h"
#include "displaydriver.h"

#define TICKS_PER_SECOND	Ticks(1000)

// Current date and time in local time. Initialise to 2020-10-10
unsigned years = 2020;		// Year number
unsigned days = 283;		// No. of days since 01.01 (0..364) (365 in leap year)
char leapday;				// 1 if current year is a leap year

unsigned char hours;		// No. of hours  0..23
unsigned char mins;			// No. of minutes 0..59
unsigned char secs;			// No of seconds 0..59

//								J	F	M	A	M	J	J	A	S	O	N	D
unsigned char monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

unsigned char update_time;

void TimekeeperInit(task_t *timekeeperTask)
{
	timekeeperTask->timer = TICKS_PER_SECOND;

	leapday = isleap(years);
	if ( leapday )
		monthdays[1] = 29;
}

void Timekeeper(task_t *timekeeperTask, unsigned long elapsed)
{
	timekeeperTask->timer += TICKS_PER_SECOND;

	unsigned char dmode = display_mode & 0x0f;

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

				if ( days >= (365 + leapday) )
				{
					days = 0;
					years++;
					leapday = isleap(years);
					if ( leapday )
						monthdays[1] = 29;
					else
						monthdays[1] = 28;

					if ( dmode == mode_YYYY )
					{
						update_time = 1;
					}
				}

				if ( dmode == mode_DDMM )
				{
					update_time = 1;
				}
			}
		}

		// If mode is hh:mm, update the display when the minutes change
		if ( dmode == mode_hhmm )
		{
			update_time = 1;
		}
	}

	// If mode is mm:ss, update the display when the seconds change.
	if ( dmode == mode_mmss )
	{
		update_time = 1;
	}
}

void flash_colon()
{
	setcolon(secs & 0x01);				// Flashing colon for time display
	display_change |= change_leds;
}

void set_mmss(void)
{
	setdigitnumeric(3, secs % 10);
	setdigitnumeric(2, secs / 10);
	setdigitnumeric(1, mins % 10);
	setdigitnumeric(0, mins / 10);
	display_change |= change_digits;
}

void set_hhmm(void)
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

void set_DDMM(void)
{
	unsigned d = days + 1;
	unsigned char m = 0;
	unsigned char md;

	while ( m < 12 && d > monthdays[m] )
	{
		d -= monthdays[m];
		m++;
	}
	m += 1;

	setdigitnumeric(3, m % 10);
	setdigitnumeric(2, m / 10);
	setdigitnumeric(1, d % 10);
	setdigitnumeric(0, d / 10);
	setcolon(1);
	display_change |= change_leds | change_digits;
}

void set_YYYY(void)
{
	unsigned y = years;
	setdigitnumeric(3, y % 10);
	y = y / 10;
	setdigitnumeric(2, y % 10);
	y = y / 10;
	setdigitnumeric(1, y % 10);
	y = y / 10;
	setdigitnumeric(0, y % 10);
	setcolon(0);
	display_change |= change_leds | change_digits;
}

void gettime(datetime_t *dt)
{
	dt->years = years;
	dt->days = days;
	dt->hours = hours;
	dt->mins = mins;
}

void settime(const datetime_t *dt)
{
	years = dt->years;
	days = dt->days;
	hours = dt->hours;
	mins = dt->mins;
	secs = 0;
	leapday = isleap(years);

	// ToDo: synchronize the task interval
}

char isleap(unsigned y)
{
	if ( (y % 4) == 0 )
	{
		if ( (y % 100) == 0 )
		{
			if ( (y % 400) == 0 )
				return 1;
			return 0;
		}
		return 1;
	}
	return 0;
}
