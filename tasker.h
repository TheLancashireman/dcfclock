/* tasker.h - an arduino round-robin time-triggered scheduler
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
 *
 *
*/
#ifndef TASKER_H
#define TASKER_H	1

typedef struct task_s task_t;
typedef void (*taskinit_t)(task_t *);
typedef void (*taskrun_t)(task_t *, unsigned long);
struct task_s
{
	taskinit_t initFunc;
	taskrun_t runFunc;
	unsigned timer;
};

void taskerSetup(task_t taskList[], int nTasks);
void taskerRun(task_t taskList[], int nTasks, unsigned (*readtime)(void));

#endif
