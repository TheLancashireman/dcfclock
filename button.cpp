/* button.cpp - monitor the pushbuttons and manage the display mode/state
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
#include "button.h"
#include "displaydriver.h"
#include "timekeeper.h"
#include "setting.h"

#define SCAN_INTERVAL	Ticks(20)

#define OneSecond		Ticks(1000)/SCAN_INTERVAL
#define NORMAL_TIMEOUT	(OneSecond * 5)
#define SETTING_TIMEOUT	(OneSecond * 10)

#define ModeBtn			8
#define UpBtn			6
#define DownBtn			7

// Button states - so it's possible to translate a mix of n/c and n/o switches
#define PRESSED			1
#define RELEASED		0

// Previous values of the buttons
static char mode_btn_prev = RELEASED;
static char up_btn_prev = RELEASED;
static char down_btn_prev = RELEASED;
static int timeout_counter;

#if DBG
static char mode_btn_dbg = RELEASED;
static char up_btn_dbg = RELEASED;
static char down_btn_dbg = RELEASED;

#define DBG_PRINT(x)	Serial.println(x)

void btn_debug(char m, char u, char d)
{
	if ( m != mode_btn_dbg )
	{
		mode_btn_dbg = m;
		DBG_PRINT(m == PRESSED ? "mode PRESSED" : "mode RELEASED");
	}

	if ( u != up_btn_dbg )
	{
		up_btn_dbg = u;
		DBG_PRINT(u == PRESSED ? "up PRESSED" : "up RELEASED");
	}

	if ( d != down_btn_dbg )
	{
		down_btn_dbg = d;
		DBG_PRINT(d == PRESSED ? "down PRESSED" : "down RELEASED");
	}
}

#else
#define DBG_PRINT(x)		do { } while (0)
#define btn_debug(m, u, d)	do { } while (0)
#endif

void check_timeout(void);
void toggle_state(void);
void toggle_setting(void);
void advance_mode(void);

void ButtonInit(task_t *buttonTask)
{
	buttonTask->timer = SCAN_INTERVAL;

	pinMode(ModeBtn, INPUT_PULLUP);
	pinMode(UpBtn, INPUT_PULLUP);
	pinMode(DownBtn, INPUT_PULLUP);

	timeout_counter = OneSecond;		// Switch to normal hhmm mode after 1 sec
}

void Button(task_t *buttonTask, unsigned long elapsed)
{
	buttonTask->timer += SCAN_INTERVAL;

	// Read all the buttons
	char mode_btn_new =	( digitalRead(ModeBtn) == HIGH ) ? RELEASED : PRESSED;
	char up_btn_new =	( digitalRead(UpBtn) == HIGH )   ? RELEASED : PRESSED;
	char down_btn_new =	( digitalRead(DownBtn) == HIGH ) ? RELEASED : PRESSED;

	btn_debug(mode_btn_new, up_btn_new, down_btn_new);

	if ( mode_btn_new == RELEASED && up_btn_new == RELEASED && down_btn_new == RELEASED )
	{
		// No buttons pressed; on timeout, return to base state.
		check_timeout();
	}
	else
	{
		if ( mode_btn_new == PRESSED )
		{
			if ( up_btn_new == PRESSED && up_btn_prev == RELEASED )
			{
				// Mode held and up button newly pressed
				// Toggle between normal and off state, or leave the setting state without change.
				toggle_state();
			}
			else if ( down_btn_new == PRESSED && down_btn_prev == RELEASED )
			{
				// Mode held and down button newly pressed
				// Enter/leave setting state (set time on leaving)
				toggle_setting();
			}
			else if ( mode_btn_prev == RELEASED )
			{
				// Mode button newly pressed on its own
				// Advannce to next mode or digit
				if ( (display_mode & 0xf0) == state_setting )
				{
					// Advance to next digit in setting mode
					advance_setting();
				}
				else
				{
					// Advance to next mode
					advance_mode();
				}
			}
		}
		else if ( up_btn_new == PRESSED && up_btn_prev == RELEASED )
		{
			if ( (display_mode & 0xf0) == state_setting )
				increase_digit();
		}
		else if ( down_btn_new == PRESSED && down_btn_prev == RELEASED )
		{
			if ( (display_mode & 0xf0) == state_setting )
				decrease_digit();
		}

		if ( (display_mode & 0xf0) == state_setting )
		{
			timeout_counter = SETTING_TIMEOUT;
		}
		else
		{
			timeout_counter = NORMAL_TIMEOUT;
		}
	}

	mode_btn_prev = mode_btn_new;
	up_btn_prev = up_btn_new;
	down_btn_prev = down_btn_new;
}

// check_timeout() - check for a timeout and revert to previous running state
void check_timeout(void)
{
	// No buttons pressed; on timeout, return to base state.
	if ( timeout_counter > 0 )
	{
		timeout_counter--;
		if ( timeout_counter <= 0 )
		{
			// If the display isn't showing normal hh:mm, clear it.
			// Among other things, this clears out any unnecessary punctuation.
			if ( display_mode != ( state_normal | mode_hhmm ) )
			{
				setdigit(0, 0x00);
				setdigit(1, 0x00);
				setdigit(2, 0x00);
				setdigit(3, 0x00);
				setdigit(4, 0x00);
			}

			unsigned char state = display_mode & 0xf0;
			if ( state == state_off )
			{
				DBG_PRINT("Revert to off state");
				display_mode = state_off | mode_xxx;
			}
			else
			{
				DBG_PRINT("Revert to normal state");
				if ( state == state_setting )
					dps_off();
				display_mode = state_normal | mode_hhmm;
			}
			display_change |= change_digits | change_leds;
			update_time = 1;
		}
	}
}

// toggle_state() - switch between normal and off states
// If in setting state, go to normal
void toggle_state(void)
{
	// Toggle between normal and off state.
	// If in the setting state, return to normal state without changing time.
	unsigned char state = display_mode & 0xf0;
	if ( state == state_normal )
	{
		DBG_PRINT("Switch to off state");
		display_mode = state_off | mode_xxx;
		// Blank the display
		setdigit(0, 0x00);
		setdigit(1, 0x00);
		setdigit(2, 0x00);
		setdigit(3, 0x00);
		setdigit(4, 0x00);
		display_change |= change_digits | change_leds;
	}
	else
	{
		if ( state == state_setting )
			dps_off();
		DBG_PRINT("Switch to normal state");
		display_mode = state_normal | mode_hhmm;
		update_time = 1;
	}
}

// toggle_setting() - switch between normal/off and setting state
// When switching from setting to normal, update the time
void toggle_setting(void)
{
	// Enter/leave setting state (set time on leaving)
	if ( (display_mode & 0xf0) == state_setting )
	{
		DBG_PRINT("Leave setting");
		leave_setting();
	}
	else
	{
		DBG_PRINT("Enter setting");
		enter_setting();
	}
}

// advance_mode() - change to next mode in sequence
void advance_mode(void)
{
	DBG_PRINT("mode++");
	unsigned char mode = display_mode & 0x0f;
	unsigned char state = display_mode & 0xf0;

	if ( mode >= mode_xxx )
	{
		display_mode = state | mode_hhmm;
		blank();	// Clear out the junk from test mode
	}
	else
	{
		display_mode++;
		if ( display_mode == ( state_normal | mode_xxx ) )
		{
			// All LEDs on. Colon will blink.
			allon();
		}
		else if ( display_mode == ( state_off | mode_xxx ) )
		{
			// Blank the display
			blank();
		}
	}
	update_time = 1;
}
