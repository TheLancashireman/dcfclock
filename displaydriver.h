/* displaydriver.h - maintain the display
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
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H		1

// 5 digits: 4 real digits plus a set of assorted LEDs
#define nDigits		5

// Digits 0 to 3
#define seg_dp		0x01	// Right-hand decimal point
#define seg_a		0x02
#define seg_b		0x04
#define seg_c		0x08
#define seg_d		0x10
#define seg_e		0x20
#define seg_f		0x40
#define seg_g		0x80

// "Digit" 4
#define seg_ldp4	0x01	// Left-hand decimal point, 1st digit
#define seg_ldp3	0x02	// Left-hand decimal point, 2nd digit
#define seg_ldp2	0x04	// Left-hand decimal point, 3rd digit
#define seg_ldp1	0x08	// Left-hand decimal point, 4th digit
#define	seg_col_u	0x10	// Colon: upper LED
#define seg_col_l	0x20	// Colon: lower LED
#define seg_aux1	0x40	// Aux1 LED (purpose TBD)
#define seg_aux2	0x80	// Aux2 LED (purpose TBD)

// Character generation
#define chargen_0	(seg_a|seg_b|seg_c|seg_d|seg_e|seg_f)
#define chargen_1	(seg_b|seg_c)
#define chargen_2	(seg_a|seg_b|seg_g|seg_e|seg_d)
#define chargen_3	(seg_a|seg_b|seg_g|seg_c|seg_d)
#define chargen_4	(seg_f|seg_g|seg_b|seg_c)
#define chargen_5	(seg_a|seg_f|seg_g|seg_c|seg_d)
#define chargen_6	(seg_a|seg_f|seg_e|seg_d|seg_c|seg_g)
#define chargen_7	(seg_a|seg_b|seg_c)
#define chargen_8	(seg_a|seg_b|seg_c|seg_d|seg_e|seg_f|seg_g)
#define chargen_9	(seg_g|seg_f|seg_a|seg_b|seg_c|seg_d)
#define chargen_a	(seg_g|seg_c|seg_d|seg_e)
#define chargen_b	(seg_f|seg_e|seg_d|seg_c|seg_g)
#define chargen_c	(seg_g|seg_e|seg_d)
#define chargen_d	(seg_b|seg_c|seg_d|seg_e|seg_g)
#define chargen_e	(seg_a|seg_f|seg_g|seg_e|seg_d)
#define chargen_f	(seg_a|seg_f|seg_g|seg_e)

// Update requests
#define change_leds		0x01
#define change_digits	0x02
#define change_all		(change_leds|change_digits)

// Check for update requests every 100 ms
#define ddInterval	100

// Display modes
#define mode_mmss	0		// Time mode
#define mode_hhmm	1		// Time mode
#define mode_DDMM	2		// Date mode
#define mode_YYYY	3		// Date mode

extern unsigned char display[nDigits];
extern unsigned char display_change;
extern unsigned char display_mode;
extern const unsigned char digit_to_7seg[16];

// The two tasker functions
void DisplayDriverInit(task_t *);
void DisplayDriver(task_t *, unsigned long elapsed);

// setdigitsegments() - sets the segments a..g to specified values (leaves dp alone)
static inline void setdigitsegments(int dig, unsigned char segs)
{
	display[dig] = (display[dig] & seg_dp) | segs;
}

// setdigitdp() - sets the (right-hand) dp to specified value (leaves a..g alone)
static inline void setdigitdp(int dig, unsigned char dp)
{
	if ( dp )
		display[dig] |= seg_dp;
	else
		display[dig] &= ~seg_dp;
}

// setdigitnumeric() - sets the segments a..g to a specified hexadecimal digit (leaves dp alone)
static inline void setdigitnumeric(int dig, unsigned char num)
{
	switch (num)
	{
	case 0:	setdigitsegments(dig, chargen_0);	break;
	case 1:	setdigitsegments(dig, chargen_1);	break;
	case 2:	setdigitsegments(dig, chargen_2);	break;
	case 3:	setdigitsegments(dig, chargen_3);	break;
	case 4:	setdigitsegments(dig, chargen_4);	break;
	case 5:	setdigitsegments(dig, chargen_5);	break;
	case 6:	setdigitsegments(dig, chargen_6);	break;
	case 7:	setdigitsegments(dig, chargen_7);	break;
	case 8:	setdigitsegments(dig, chargen_8);	break;
	case 9:	setdigitsegments(dig, chargen_9);	break;
	default:	break;	// Nothing				
	}
}

// setcolon() - sets the colon LEDs on or off
static inline void setcolon(int on)
{
	if ( on )
		display[4] |= (seg_col_u | seg_col_l);
	else
		display[4] &= ~(seg_col_u | seg_col_l);
}

// setled() - sets a LED on or off
static inline void setled(unsigned char led, int on)
{
	if ( on )
		display[4] |= led;
	else
		display[4] &= ~led;
}

#endif
