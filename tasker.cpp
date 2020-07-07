/* tasker.cpp - an arduino round-robin time-triggered scheduler
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
#include "tasker.h"

void taskerSetup(task_t taskList[], int nTasks)
{
	for ( int i = 0; i < nTasks; i++ )
	{
		taskList[i].initFunc(&taskList[i]);
	}
}

void taskerRun(task_t taskList[], int nTasks, unsigned (*readtime)(void))
{
	unsigned then = readtime();

	for (;;)
	{
		unsigned now = readtime();
		unsigned elapsed = now - then;

		if ( elapsed > 0 )
		{
			for ( int i = 0; i < nTasks; i++ )
			{
				if ( taskList[i].timer <= elapsed )
				{
					taskList[i].runFunc(&taskList[i], elapsed);
				}

				if ( taskList[i].timer < elapsed )
				{
					/* If this branch gets executed regularly,
					 * then executing the tasks takes longer than the interval.
					*/
					taskList[i].timer = 0;
				}
				else
				{
					taskList[i].timer -= elapsed;
				}
			}
		}
		then = now;
	}
}
