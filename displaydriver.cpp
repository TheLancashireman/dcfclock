/* displaydriver.cpp - drive the display
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
#include <Arduino.h>
#include <SPI.h>
#include "dcfclock.h"
#include "tasker.h"
#include "displaydriver.h"

// Pin assginments
#define SpiClk			13			// SPI clock pin - unfortunately same as on-board LED
#define SpiMosi			11			// SPI output data (MOSI)
#define SpiMiso			12			// SPI input data (not used)

#define SrLatch4		10			// Latch the four main digits
#define SrLatch1		9			// Latch the extra LEDs (left DP, colon etc.)

#define ddInterval		Ticks(100)	// 100 ms

unsigned char display[nDigits];
unsigned char display_change;
unsigned char display_mode;

void DisplayDriverInit(task_t *displayDriveTask)
{
	pinMode(SrLatch1, OUTPUT);		// Drive LOW to HIGH to latch the "extra LEDs"
	pinMode(SrLatch4, OUTPUT);		// Drive LOW to HIGH to latch the four digits
	pinMode(SpiClk, OUTPUT);		// SPI clock
	pinMode(SpiMosi, OUTPUT);		// SPI output data
	pinMode(SpiMiso, INPUT_PULLUP);	// SPI input data (not used)

	digitalWrite(SpiClk, LOW);		// Set clock and data to known states
	digitalWrite(SpiMosi, LOW);
	digitalWrite(SrLatch1, LOW);	// Set both latch pins to inactive
	digitalWrite(SrLatch4, LOW);

	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);

	display_change = 0;
	display_mode = mode_xxx | state_normal;

	for ( int i = 0; i < nDigits; i++ )	// Clear the SRs
	{
		display[i] = 0;
		SPI.transfer(~display[i]);
	}

	setdigitsegments(0, 0xff);
	setdigitsegments(1, 0xff);
	setdigitsegments(2, 0xff);
	setdigitsegments(3, 0xff);
	setdigitsegments(4, 0xff);
	display_change |= change_digits;
	
	digitalWrite(SrLatch1, HIGH);	// Latch the cleared SRs into the outputs
	digitalWrite(SrLatch1, LOW);
	digitalWrite(SrLatch4, HIGH);
	digitalWrite(SrLatch4, LOW);

	displayDriveTask->timer = ddInterval;
}

void DisplayDriver(task_t *displayDriveTask, unsigned long elapsed)
{
	displayDriveTask->timer += ddInterval;

	switch ( display_change )
	{
	case 0x00:	// Nothing
		break;
	case change_leds:		// Only update the extra LEDs
		SPI.transfer(~display[4]);
		digitalWrite(SrLatch1, HIGH);
		digitalWrite(SrLatch1, LOW);
		break;
	default:				// Update digits or digits and extra LEDs
		for ( int i = 0; i < nDigits; i++ )
			SPI.transfer(~display[i]);
		digitalWrite(SrLatch4, HIGH);
		digitalWrite(SrLatch4, LOW);
		if ( display_change == change_all )
		{
			digitalWrite(SrLatch1, HIGH);
			digitalWrite(SrLatch1, LOW);
		}
		break;
	}
	display_change = 0;
}
