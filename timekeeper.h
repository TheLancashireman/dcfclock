/* timekeeper.h - maintain the time and date
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
#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H	1

#include "tasker.h"

// Structure parameter for gettime() and settime()
typedef struct
{
	unsigned years;			// Year number
	unsigned days;			// No. of days since 01.01 (0..364) (365 in leap year)
	unsigned char hours;	// No. of hours  0..23
	unsigned char mins;		// No. of minutes 0..59
} datetime_t;

extern unsigned char monthdays[12];

/* Tasker init- and run functions
*/
void TimekeeperInit(task_t *);
void Timekeeper(task_t *, unsigned long elapsed);

extern void flash_colon(void);
extern void set_mmss(void);
extern void set_hhmm(void);
extern void set_DDMM(void);
extern void set_YYYY(void);

extern void gettime(datetime_t *dt);
extern void settime(const datetime_t *dt);

extern char isleap(unsigned years);

#endif
