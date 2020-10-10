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

// enter_setting() - enter setting state
// Get current date and time as starting point
void enter_setting(void)
{
	display_mode = state_setting | mode_hhmm;
}

// leave_setting() - leave setting state (go to normal state, not off)
// Set date and time
void leave_setting(void)
{
	display_mode = state_normal | mode_hhmm;
	update_time = 1;
}

// advance_setting() - change to next digit to set
void advance_setting(void)
{
}

// increase_digit() - increase the current digit by 1
// Take account of overflow (wrap around)
void increase_digit(void)
{
}

// increase_digit() - increase the current digit by 1
// Take account of underflow (wrap around)
void decrease_digit(void)
{
}
