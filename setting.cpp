/* setting.cpp - set the time and date using the buttons
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
#include "setting.h"
#include "displaydriver.h"
#include "timekeeper.h"

static datetime_t dt;
static unsigned char d[4];
static unsigned char maxd[4];
static unsigned char d_index;

static void decode_time(void);
static void decode_days(void);
static void decode_year(void);
static void encode_time(void);
static void encode_days(void);
static void encode_year(void);
static void display_digits(void);

// enter_setting() - enter setting state
// Get current date and time as starting point
void enter_setting(void)
{
	display_mode = state_setting | mode_hhmm;
	gettime(&dt);
	d_index = 0;
	decode_time();
	display_digits();
}

// leave_setting() - leave setting state (go to normal state, not off)
// Set date and time
void leave_setting(void)
{
	switch ( display_mode )
	{
	case (state_setting | mode_hhmm):
		encode_time();
		break;

	case (state_setting | mode_DDMM):
		encode_days();
		break;

	default:
		encode_year();
		break;
	}
	settime(&dt);
	display_mode = state_normal | mode_hhmm;
	update_time = 1;
}

// advance_setting() - change to next digit to set
void advance_setting(void)
{
	d_index++;
	if ( d_index > 3 )
	{
		switch ( display_mode )
		{
		case (state_setting | mode_hhmm):
			encode_time();
			display_mode = state_setting | mode_DDMM;
			d_index = 0;
			decode_days();
			display_digits();
			setcolon(1);
			break;

		case (state_setting | mode_DDMM):
			encode_days();
			display_mode = state_setting |mode_YYYY;
			d_index = 0;
			decode_year();
			display_digits();
			setcolon(0);
			break;

		default:
			encode_year();
			display_mode = state_setting | mode_hhmm;
			d_index = 0;
			decode_time();
			display_digits();
			break;
		}
	}
}

// increase_digit() - increase the current digit by 1
// Take account of overflow (wrap around)
void increase_digit(void)
{
	d[d_index]++;
	if ( d[d_index] > maxd[d_index] )
		d[d_index] = 0;
	setdigitnumeric(d_index, d[d_index]);
	display_change |= change_digits;
}

// increase_digit() - increase the current digit by 1
// Take account of underflow (wrap around)
void decrease_digit(void)
{
	if ( d[d_index] > 0 )
		d[d_index]--;
	else
		d[d_index] = maxd[d_index];;
	setdigitnumeric(d_index, d[d_index]);
	display_change |= change_digits;
}

static void decode_time(void)
{
	d[0] = dt.hours / 10;
	d[1] = dt.hours % 10;
	d[2] = dt.mins / 10;
	d[3] = dt.mins % 10;
	maxd[0] = 2;
	maxd[1] = 9;
	maxd[2] = 5;
	maxd[3] = 9;
}

static void decode_days(void)
{
	unsigned D = dt.days + 1;
	unsigned char M = 0;

	if ( D > monthdays[0] )
	{
		D -= monthdays[0];
		M = 1;

		unsigned feb = isleap(dt.years) ? 29 : 28;
		if ( D > feb )
		{
			D -= feb;
			M = 2;
		}

    	while ( M < 12 && D > monthdays[M] )
    	{
			D -= monthdays[M];
			M++;
		}
	}
	M += 1;

	d[0] = D / 10;
	d[1] = D % 10;
	d[2] = M / 10;
	d[3] = M % 10;
	maxd[0] = 3;
	maxd[1] = 9;
	maxd[2] = 1;
	maxd[3] = 9;
}

static void decode_year(void)
{
	unsigned y = dt.years;
	d[3] = y % 10;
	y = y / 10;
	d[2] = y % 10;
	y = y / 10;
	d[1] = y % 10;
	y = y / 10;
	d[0] = y % 10;
	maxd[0] = 9;
	maxd[1] = 9;
	maxd[2] = 9;
	maxd[3] = 9;
}

static void display_digits(void)
{
	setdigitnumeric(0, d[0]);
	setdigitnumeric(1, d[1]);
	setdigitnumeric(2, d[2]);
	setdigitnumeric(3, d[3]);
	display_change |= change_digits;
}

static void encode_time(void)
{
	dt.hours = d[0]*10 + d[1];
	dt.mins = d[2]*10 + d[3];
}

static void encode_days(void)
{
	unsigned char M, D;

	D = d[0]*10 + d[1];
	M = d[2]*10 + d[3];

	// Constrain ...
	if ( M < 1 )
		M = 1;
	else if ( M > 12 )
		M = 12;
	M -= 1;

	if ( D < 1 )
		D = 1;
	else if ( M == 1 )
	{
		// Special case for February
		if ( isleap(dt.years) )
		{
			if ( D > 29 )
				D = 29;
		}
		else if ( D > 28 )
			D = 28;
	}
	else if ( D > monthdays[M] )
		D = monthdays[M];

	dt.days = D - 1;
	for ( int i = 0; i < M; i++ )
		dt.days += monthdays[i];
}

static void encode_year(void)
{
	dt.years = 0;
	for ( int i = 0; i < 4; i++ )
		dt.years = dt.years * 10 + d[i];
}
