/* dcfclock - a DFC-77 clock using an Arduino Nano
 *
 * (c) 2016 David Haworth
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
 * $Id$
 *
 * dcfclock is an Arduino sketch, written for an Arduino Nano
 *
 * See https://en.wikipedia.org/wiki/DCF77
 *
 * Byte no. Bit no.  DCF bit     Description
 *      0   0        :00  M      Start of minute. Always 0.
 *      0   1        :01  1      Civil warning bits
 *      0   2        :02  1
 *      0   3        :03  1
 *      0   4        :04  1
 *      0   5        :05  1
 *      0   6        :06  1
 *      0   7        :07  1
 *      1   0        :08  1
 *      1   1        :09  1
 *      1   2        :10  0
 *      1   3        :11  0
 *      1   4        :12  0
 *      1   5        :13  0
 *      1   6        :14  0
 *      1   7        :15  R      Call bit: abnormal transmitter operation.
 *      2   0        :16  A1     Summer time announcement. Set during hour before change.
 *      2   1        :17  Z1     Set to 1 when CEST is in effect.
 *      2   2        :18  Z2     Set to 1 when CET is in effect.
 *      2   3        :19  A2     Leap second announcement.  Set during hour before leap second.
 *      2   4        :20  S      Start of encoded time. Always 1.
 *      2   5        :21  1      Minutes
 *      2   6        :22  2
 *      2   7        :23  4
 *      3   0        :24  8
 *      3   1        :25  10
 *      3   2        :26  20
 *      3   3        :27  40
 *      3   4        :28  P1     Even parity over minute bits
 *      3   5        :29  1      Hours
 *      3   6        :30  2
 *      3   7        :31  4
 *      4   0        :32  8
 *      4   1        :33  10
 *      4   2        :34  20
 *      4   3        :35  P2     Even parity over hour bits
 *      4   4        :36  1      Day of month.
 *      4   5        :37  2
 *      4   6        :38  4
 *      4   7        :39  8
 *      5   0        :40  10
 *      5   1        :41  20
 *      5   2        :42  1      Day of week Monday=1, Sunday=7
 *      5   3        :43  2
 *      5   4        :44  4
 *      5   5        :45  1      Month number
 *      5   6        :46  2
 *      5   7        :47  4
 *      6   0        :48  8
 *      6   1        :49  10
 *      6   2        :50  1      Year within century
 *      6   3        :51  2
 *      6   4        :52  4
 *      6   5        :53  8
 *      6   6        :54  10
 *      6   7        :55  20
 *      7   0        :56  40
 *      7   1        :57  80
 *      7   2        :58  P3     Even parity over date bits
 *      7   3        :59  0      Minute mark: no amplitude modulation
*/

typedef struct task_s task_t;
typedef void (*taskinit_t)(task_t *);
typedef void (*taskrun_t)(task_t *, unsigned long);
struct task_s
{
	unsigned long timer;
	taskinit_t initFunc;
	taskrun_t runFunc;
};

/* Pin assginments
*/
#define LedPin			13		// On-board LED connected to digital pin 13
#define DcfInputPin		8		// PB0 - DCF receiver output connected to this
#define DcfPonPin		9		// PB1 - DCF receiver PON input connected to this

#define SegPin_a		14		// PC0
#define SegPin_b		15		// PC1
#define SegPin_c		2		// PD2
#define SegPin_d		3		// PD3
#define SegPin_e		4		// PD4
#define SegPin_f		5		// PD5
#define SegPin_g		6		// PD6
#define SegPin_h		7		// P76
#define DigPin_0		10		// PB2
#define DigPin_1		11		// PB3
#define DigPin_2		12		// PB4

/* Timed processing
*/
unsigned long then;

/* Tasks
 *
 * Each "Task" has an init function and a run function
 *    - the init function is called once at startup
 *    - the run function is called once every millisecond
*/
void LedPingInit(task_t *);
void LedPing(task_t *, unsigned long elapsed);

void DcfSampleInit(task_t *);
void DcfSample(task_t *, unsigned long elapsed);

void DisplayDriveInit(task_t *);
void DisplayDrive(task_t *, unsigned long elapsed);
void SetDigitBit(unsigned char digit, unsigned char bit, unsigned char val);
void SetDigit(unsigned char digit, unsigned char val);
void SetDigit7Seg(unsigned char digit, unsigned char val);

/* Task list
*/
#define NTASKS	3
task_t taskList[NTASKS] =
{	{	0,	LedPingInit, 		LedPing			},
	{	0,	DcfSampleInit,		DcfSample		},
	{	0,	DisplayDriveInit,	DisplayDrive	}
};

/* setup() - standard Arduino startup function
*/
void setup(void)
{
	unsigned char i;

	for ( i = 0; i < NTASKS; i++ )
	{
		taskList[i].initFunc(&taskList[i]);
	}

	Serial.begin(9600);					// Start the serial port.
	Serial.println("Hello world!");		// ToDo : it'll need a "who are you?" response

	then = millis();					// Initialise the time reference.
}

/* loop() - standard Arduino run function
*/
void loop(void)
{
	unsigned char i;
	unsigned long now = millis();
	unsigned long elapsed = now - then;

	if ( elapsed > 0 )
	{
		for ( i = 0; i < NTASKS; i++ )
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

unsigned char calcParity(unsigned char x)
{
	unsigned char p = x;
	p = (p ^ (p >> 4));
	p = (p ^ (p >> 2));
	p = (p ^ (p >> 1)) & 0x01;
	return p;
}


/* ========================================
 * LED Ping task
*/
#define ledPingOnInterval	20
#define ledPingOffInterval	1980
char ledState;

void LedPingInit(task_t *ledTask)
{
	pinMode(LedPin, OUTPUT);			// Sets the LED pin as output
	digitalWrite(LedPin, LOW);			// Drive the pin low (LED off)
	ledState = 0;
	ledTask->timer = ledPingOffInterval;
}

void LedPing(task_t *ledTask, unsigned long elapsed)
{
	if ( ledState )
	{
		/* Turn LED off
		*/
		ledState = 0;
		digitalWrite(LedPin, LOW);
		ledTask->timer += ledPingOffInterval;
	}
	else
	{
		/* Turn LED on
		*/
		ledState = 1;
		digitalWrite(LedPin, HIGH);
		ledTask->timer += ledPingOnInterval;
	}
}

/* ========================================
 * DCF Sample task
*/
#define dcfSampleInterval	5		//	5 milliseconds
#define dcfDebounce			50		//	50 milliseconds
#define dcfPowerOnInterval	1100	//	1.1 seconds
#define dcfPulseWidthZero	100		//	100 milliseconds pulse width --> binary 0
#define dcfPulseWidthOne	200		//	200 milliseconds pulse width --> binary 1
#define dcfPulseWidthThrsh	((dcfPulseWidthZero+dcfPulseWidthOne)/2)
#define dcfPulseInterval	1000	//	Period of DCF pulses
#define dcfState0			LOW
#define dcfState1			HIGH
unsigned char dcfState = 3;
unsigned dcfWidth = 0;
unsigned dcfGapThrsh = (dcfPulseInterval+dcfDebounce-dcfPulseWidthZero);
unsigned char dcfShiftReg[8] = {0,0,0,0,0,0,0,0};
unsigned char dcfBitCount = 0;

/* Extracted time and date.
*/
unsigned char dcfSecond;
unsigned char dcfMinute;
unsigned char dcfHour;
unsigned char dcfDay;
unsigned char dcfMonth;
unsigned char dcfYear;
unsigned char dcfDow;

void dcfPush(unsigned char bitval);
void dcfCalculateTime(void);
void dcfWriteTime(void);

void DcfSampleInit(task_t *dcfTask)
{
	pinMode(DcfInputPin, INPUT_PULLUP); //	Set up the input pins
	pinMode(DcfPonPin, OUTPUT); 
	digitalWrite(DcfPonPin, HIGH);		// Drive the pin high (DCF off)
	dcfTask->timer = dcfPowerOnInterval;
}

void DcfSample(task_t *dcfTask, unsigned long elapsed)
{
	dcfWidth += dcfSampleInterval;
	if ( dcfState == 0 )
	{
		/* Sample the state of the DCF input.
		*/
		if ( digitalRead(DcfInputPin) == dcfState1 && dcfWidth > dcfDebounce )
		{
			/* Change of state. Start measuring the width of the pulse.
			*/
			dcfState = 1;
			dcfWidth = 0;
			dcfSecond++;
			if ( (dcfSecond & 0xf) > 9 )
			{
				// BCD adjust
				dcfSecond += 6;
			}
			SetDigitBit(4, 3, 1);
			SetDigitBit(4, 4, 1);
// Temp
			SetDigitBit(0, 7, 1);
			SetDigitBit(1, 7, 0);
			SetDigitBit(2, 7, 1);
			SetDigitBit(3, 7, 0);
		}
		else
		if ( dcfWidth > dcfGapThrsh )
		{
			/* No leading edge seen --> it's a gap.
			 * Leading edge of next pulse is start of a new minute.
			*/
			dcfState = 2;
			dcfWidth = 0;
			dcfCalculateTime();
		}
	}
	else
	if ( dcfState == 1 )
	{
		/* Sample the state of the DCF input.
		*/
		if ( digitalRead(DcfInputPin) == dcfState0 && dcfWidth > dcfDebounce )
		{
			/* Change of state. Push the bit value into the shift register.
			*/
			dcfPush((dcfWidth > dcfPulseWidthThrsh) ? 1 : 0);
			dcfState = 0;
			dcfWidth = 0;
			SetDigitBit(4, 3, 0);
			SetDigitBit(4, 4, 0);
// Temp
			SetDigitBit(0, 7, 0);
			SetDigitBit(1, 7, 1);
			SetDigitBit(2, 7, 0);
			SetDigitBit(3, 7, 1);
		}
	}
	else
	if ( dcfState == 2 )
	{
		/* Waiting for start of minute.
		*/
		if ( digitalRead(DcfInputPin) == dcfState1 )
		{
			/* Change of state. Start measuring the width of the pulse.
			*/
			unsigned char i;

			dcfState = 1;
			dcfWidth = 0;
			dcfWriteTime();
			dcfBitCount = 0;
			dcfSecond = 0;
			for ( i = 0; i < 8; i++ )
			{
				dcfShiftReg[i] = 0;
			}
		}
	}
	else
	{
		/* End of delay to allow DCF to stabilise; turn it on.
		*/
		dcfState = 0;
		dcfWidth = 0;
		digitalWrite(DcfPonPin, LOW);		// Drive the pin low (DCF on)
	}
	dcfTask->timer += dcfSampleInterval;
}

void dcfPush(unsigned char bitval)
{
	Serial.print(bitval?"1":"0");
	if ( bitval != 0 )
	{
		dcfShiftReg[dcfBitCount/8] |= ( 1 << (dcfBitCount%8) );
	}
	dcfBitCount++;
}

const char hexDigit[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
const char *strDay[8] = { "??", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };

void dcfCalculateTime(void)
{
	unsigned char pErr;

	if ( dcfBitCount == 59 )
	{
		if ( (dcfShiftReg[0] & 0x01 != 0) ||
			 (dcfShiftReg[2] & 0x10 == 0 ) )
		{
			// Error in fixed bits
			Serial.println("F");
		}
		else
		{
			dcfMinute	= ((dcfShiftReg[2] >> 5) & 0x07) | ((dcfShiftReg[3] << 3) & 0x78);
			dcfHour		= ((dcfShiftReg[3] >> 5) & 0x07) | ((dcfShiftReg[4] << 3) & 0x38);
			dcfDay		= ((dcfShiftReg[4] >> 4) & 0x0f) | ((dcfShiftReg[5] << 4) & 0x30);
			dcfMonth	= ((dcfShiftReg[5] >> 5) & 0x07) | ((dcfShiftReg[6] << 3) & 0x18);
			dcfYear		= ((dcfShiftReg[6] >> 2) & 0x3f) | ((dcfShiftReg[7] << 6) & 0xc0);
			dcfDow		= ((dcfShiftReg[5] >> 2) & 0x07);
			pErr = calcParity(dcfMinute) ^ ((dcfShiftReg[3] & 0x10) != 0);
			pErr |= (calcParity(dcfHour) ^ ((dcfShiftReg[4] & 0x08) != 0)) << 1;
			pErr |= (calcParity(dcfDay^dcfMonth^dcfYear^dcfDow) ^ ((dcfShiftReg[7] & 0x04) != 0)) << 2;

			if ( pErr )
			{
				if ( pErr & 1 )	Serial.print('M');
				if ( pErr & 2 )	Serial.print('H');
				if ( pErr & 4 )	Serial.print('D');
				Serial.println('P');
			}
			else
			{
				Serial.println('0');
			}

			Serial.print(strDay[dcfDow]);
			Serial.print(' ');
			Serial.print("20");
			Serial.print(hexDigit[dcfYear>>4]);
			Serial.print(hexDigit[dcfYear&0xf]);
			Serial.print('-');
			Serial.print(hexDigit[dcfMonth>>4]);
			Serial.print(hexDigit[dcfMonth&0xf]);
			Serial.print('-');
			Serial.print(hexDigit[dcfDay>>4]);
			Serial.print(hexDigit[dcfDay&0xf]);
			Serial.print(' ');
			Serial.print(hexDigit[dcfHour>>4]);
			Serial.print(hexDigit[dcfHour&0xf]);
			Serial.print(':');
			Serial.print(hexDigit[dcfMinute>>4]);
			Serial.println(hexDigit[dcfMinute&0xf]);

		}
	}
	else
	if ( dcfBitCount < 59 )
	{
		Serial.println("-");
	}
	else
	{
		Serial.println("+");
	}
}

void dcfWriteTime(void)
{
	SetDigit7Seg(0, dcfHour/16);
	SetDigit7Seg(1, dcfHour%16);
	SetDigit7Seg(2, dcfMinute/16);
	SetDigit7Seg(3, dcfMinute%16);
}


/* ========================================
 * Display drive task
*/
#define ddInterval	1

#define seg_a	0x01
#define seg_b	0x02
#define seg_c	0x04
#define seg_d	0x08
#define seg_e	0x10
#define seg_f	0x20
#define seg_g	0x40
#define seg_h	0x80

#define nDigits		5
unsigned char digit7seg[nDigits];
unsigned char dCount;

const unsigned char digit_to_7seg[16] =
{	/* 0 */	(seg_a|seg_b|seg_c|seg_d|seg_e|seg_f),
	/* 1 */	(seg_b|seg_c),
	/* 2 */	(seg_a|seg_b|seg_g|seg_e|seg_d),
	/* 3 */	(seg_a|seg_b|seg_g|seg_c|seg_d),
	/* 4 */	(seg_f|seg_g|seg_b|seg_c),
	/* 5 */	(seg_a|seg_f|seg_g|seg_c|seg_d),
	/* 6 */	(seg_a|seg_f|seg_e|seg_d|seg_c|seg_g),
	/* 7 */	(seg_a|seg_b|seg_c),
	/* 8 */	(seg_a|seg_b|seg_c|seg_d|seg_e|seg_f|seg_g),	/* all on */
	/* 9 */	(seg_g|seg_f|seg_a|seg_b|seg_c|seg_d),
	/* a */	(seg_g|seg_c|seg_d|seg_e),
	/* b */	(seg_f|seg_e|seg_d|seg_c|seg_g),
	/* c */	(seg_g|seg_e|seg_d),
	/* d */	(seg_b|seg_c|seg_d|seg_e|seg_g),
	/* e */	(seg_a|seg_f|seg_g|seg_e|seg_d),
	/* f */	(seg_a|seg_f|seg_g|seg_e)
};

const unsigned char segpins[8] =
{	SegPin_a, SegPin_b, SegPin_c, SegPin_d, SegPin_e, SegPin_f, SegPin_g, SegPin_h	};

void DisplayDriveInit(task_t *displayDriveTask)
{
	pinMode(SegPin_a, OUTPUT);			// Sets the segment pins as output
	pinMode(SegPin_b, OUTPUT);
	pinMode(SegPin_c, OUTPUT);
	pinMode(SegPin_d, OUTPUT);
	pinMode(SegPin_e, OUTPUT);
	pinMode(SegPin_f, OUTPUT);
	pinMode(SegPin_g, OUTPUT);
	pinMode(SegPin_h, OUTPUT);
	pinMode(DigPin_0, OUTPUT);
	pinMode(DigPin_1, OUTPUT);
	pinMode(DigPin_2, OUTPUT);

	digitalWrite(SegPin_a, LOW);		// Drive the segment pins low (LED off)
	digitalWrite(SegPin_b, LOW);
	digitalWrite(SegPin_c, LOW);
	digitalWrite(SegPin_d, LOW);
	digitalWrite(SegPin_e, LOW);
	digitalWrite(SegPin_f, LOW);
	digitalWrite(SegPin_g, LOW);
	digitalWrite(SegPin_h, LOW);
	digitalWrite(SegPin_h, LOW);
	digitalWrite(SegPin_h, LOW);
	digitalWrite(DigPin_0, LOW);
	digitalWrite(DigPin_1, LOW);
	digitalWrite(DigPin_2, LOW);

	digit7seg[0] = seg_g|seg_h;
	digit7seg[1] = seg_g|seg_h;
	digit7seg[2] = seg_g|seg_h;
	digit7seg[3] = seg_g|seg_h;

	displayDriveTask->timer = ddInterval;
}

void DisplayDrive(task_t *displayDriveTask, unsigned long elapsed)
{
	unsigned segvals = digit7seg[dCount];

	digitalWrite(DigPin_0, HIGH);
	digitalWrite(DigPin_1, HIGH);
	digitalWrite(DigPin_2, HIGH);
	digitalWrite(SegPin_a, ( segvals & 0x01 ) ? HIGH : LOW);
	digitalWrite(SegPin_b, ( segvals & 0x02 ) ? HIGH : LOW);
	digitalWrite(SegPin_c, ( segvals & 0x04 ) ? HIGH : LOW);
	digitalWrite(SegPin_d, ( segvals & 0x08 ) ? HIGH : LOW);
	digitalWrite(SegPin_e, ( segvals & 0x10 ) ? HIGH : LOW);
	digitalWrite(SegPin_f, ( segvals & 0x20 ) ? HIGH : LOW);
	digitalWrite(SegPin_g, ( segvals & 0x40 ) ? HIGH : LOW);
	digitalWrite(SegPin_h, ( segvals & 0x80 ) ? HIGH : LOW);
	digitalWrite(DigPin_0, ( dCount & 0x01 ) ? HIGH : LOW);
	digitalWrite(DigPin_1, ( dCount & 0x02 ) ? HIGH : LOW);
	digitalWrite(DigPin_2, ( dCount & 0x04 ) ? HIGH : LOW);

	dCount++;

	if ( dCount >= nDigits )
	{
		dCount = 0;
	}

	displayDriveTask->timer += ddInterval;
}

void SetDigit(unsigned char digit, unsigned char val)
{
	if ( digit < nDigits )
	{
		digit7seg[digit] = val;
	}
}

void SetDigitBit(unsigned char digit, unsigned char bit, unsigned char val)
{
	if ( (digit < nDigits) && (bit < 8) )
	{
		if ( val )
		{
			digit7seg[digit] |= (1 << bit);
		}
		else
		{
			digit7seg[digit] &= ~(1 << bit);
		}
	}
}

void SetDigit7Seg(unsigned char digit, unsigned char val)
{
	if ( digit < nDigits )
	{
		digit7seg[digit] = (digit7seg[digit] & 0x80) | digit_to_7seg[val&0x0f];
	}
}