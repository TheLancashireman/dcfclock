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

#define SCAN_INTERVAL	Ticks(20)

#define OneSecond		Ticks(1000)/SCAN_INTERVAL
#define NORMAL_TIMEOUT	(OneSecond * 10)
#define SETTING_TIMEOUT	(OneSecond * 70)

#define ModeBtn			8
#define UpBtn			6
#define DownBtn			7

// Previous values of the buttons
static char mode_btn_prev = HIGH;
static char up_btn_prev = HIGH;
static char down_btn_prev = HIGH;
static int timeout_counter;

#if DBG
static char mode_btn_dbg = HIGH;
static char up_btn_dbg = HIGH;
static char down_btn_dbg = HIGH;
#endif

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
	char mode_btn_new = digitalRead(ModeBtn);
	char up_btn_new = digitalRead(UpBtn);
	char down_btn_new = digitalRead(DownBtn);

#if DBG
	if ( mode_btn_new != mode_btn_dbg )
	{
		mode_btn_dbg = mode_btn_new;
		Serial.println(mode_btn_new == LOW ? "mode LOW" : "mode HIGH");
	}

	if ( up_btn_new != up_btn_dbg )
	{
		up_btn_dbg = up_btn_new;
		Serial.println(up_btn_new == LOW ? "up LOW" : "up HIGH");
	}

	if ( down_btn_new != down_btn_dbg )
	{
		down_btn_dbg = down_btn_new;
		Serial.println(down_btn_new == LOW ? "down LOW" : "down HIGH");
	}
#endif

	if ( mode_btn_new == HIGH && up_btn_new == HIGH && down_btn_new == HIGH )
	{
		// No buttons pressed; on timeout, return to base state.
		if ( timeout_counter > 0 )
		{
			timeout_counter--;
			if ( timeout_counter <= 0 )
			{
				if ( display_mode != ( state_normal | mode_hhmm ) )
				{
					setdigit(0, 0x00);
					setdigit(1, 0x00);
					setdigit(2, 0x00);
					setdigit(3, 0x00);
					setdigit(4, 0x00);
				}

				if ( (display_mode & 0xf0) == state_off )
				{
#if DBG
			Serial.println("Revert to off state");
#endif
					display_mode = state_off | mode_xxx;
				}
				else
				{
#if DBG
			Serial.println("Revert to normal state");
#endif
					display_mode = state_normal | mode_hhmm;
				}
				display_change |= change_digits | change_leds;
				update_time = 1;
			}
		}
	}
	else
	{
		if ( mode_btn_new == LOW )
		{
			if ( down_btn_new == LOW && down_btn_prev == HIGH )
			{
				// Enter/leave setting state (set time on leaving)
				if ( (display_mode & 0xf0) == state_setting )
				{
#if DBG
			Serial.println("Leave setting");
#endif
					// ToDo: set the new time
					display_mode = state_normal | mode_hhmm;
					update_time = 1;
				}
				else
				{
#if DBG
			Serial.println("Enter setting");
#endif
					display_mode = state_setting | mode_hhmm;
				}
			}
			else if ( up_btn_new == LOW && up_btn_prev == HIGH )
			{
				// Up button newly pressed
				// Toggle between normal and off state, or leave the setting state without change.
				unsigned char state = display_mode & 0xf0;
				if ( state == state_normal )
				{
#if DBG
			Serial.println("Switch to off state");
#endif
					display_mode = state_off | mode_xxx;
					// Blank the display
					setdigit(0, 0x00);
					setdigit(1, 0x00);
					setdigit(2, 0x00);
					setdigit(3, 0x00);
					setdigit(4, 0x00);
					display_change |= change_digits | change_leds;
				}
				else if ( state == state_off )
				{
#if DBG
			Serial.println("Switch to normal state");
#endif
					display_mode = state_normal | mode_hhmm;
				}

				update_time = 1;
			}
			else if ( mode_btn_prev == HIGH )
			{
				// Mode button newly pressed on its own
				// Advannce to next mode
				unsigned char state = display_mode & 0xf0;

				if ( state == state_setting )
				{
					// ToDo: advance to next digit
				}
				else
				{
#if DBG
			Serial.println("mode++");
#endif
					unsigned char mode = display_mode & 0x0f;
					if ( mode >= mode_xxx )
					{
						display_mode = state | mode_hhmm;

						setdigit(0, 0x00);		// Clear out the junk from test mode
						setdigit(1, 0x00);
						setdigit(2, 0x00);
						setdigit(3, 0x00);
						setdigit(4, 0x00);
					}
					else
					{
						display_mode++;
						if ( display_mode == ( state_normal | mode_xxx ) )
						{
							// All LEDs on. Colon will blink.
							setdigit(0, 0xff);
							setdigit(1, 0xff);
							setdigit(2, 0xff);
							setdigit(3, 0xff);
							setdigit(4, 0xff);
							display_change |= change_digits | change_leds;
						}
						else if ( display_mode == ( state_off | mode_xxx ) )
						{
							// Blank the display
							setdigit(0, 0x00);
							setdigit(1, 0x00);
							setdigit(2, 0x00);
							setdigit(3, 0x00);
							setdigit(4, 0x00);
							display_change |= change_digits | change_leds;
						}
					}
					update_time = 1;
				}
			}
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
