/* dcfclock.cpp - a DFC-77 clock using an Arduino Nano
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
#include <Arduino.h>
#include <SPI.h>
#include "dcfclock.h"
#include "tasker.h"
#include "timekeeper.h"
#include "displaydriver.h"
#include "dcfdecoder.h"

// Task list
#define NTASKS	3
task_t taskList[NTASKS] =
{	{	DisplayDriverInit,	DisplayDriver,	0	},
	{	TimekeeperInit,		Timekeeper,		0	},
	{	DcfDecoderInit,		DcfDecoder,		0	}
};

// setup() - standard Arduino startup function
// Everything happens in here
void setup(void)
{
	taskerSetup(taskList, NTASKS);

	Serial.begin(115200);				// Start the serial port.
	Serial.println("Hello world!");		// ToDo : it'll need a "who are you?" response

	taskerRun(taskList, NTASKS, ReadTime);
}

// loop() - standard Arduino run function (not used)
void loop(void)
{
}

// Time function for the tasker module
unsigned ReadTime(void)
{
#if TimeSource == Time_millis
	return (unsigned)millis();
#else
#error "External time not supported yet"
	return 0;
#endif
}
