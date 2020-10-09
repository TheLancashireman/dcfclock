/* dcfdecoder.cpp - receive and decode DCF77 time signal
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
#include <Arduino.h>
#include "dcfclock.h"
#include "tasker.h"
#include "displaydriver.h"

#define DcfInputPin		2			// DCF receiver output connected to this (must be an INT pin)
#define DcfPonPin		4			// DCF receiver PON input connected to this

#define DcfPonInterval	Ticks(1100)	// 1.1 seconds
#define DcfInterval		Ticks(100)	// 0.1 seconds

#define DcfMinSync		Ticks(1900)	// 1.9 seconds
#define DcfMaxSync		Ticks(2200)	// 2.2 seconds
#define DcfDebounce		Ticks(60)
#define DcfMin0			Ticks(80)	// Pulse width 100 ms +/- 20 --> 0
#define DcfMax0			Ticks(120)
#define DcfMin1			Ticks(180)	// Pulse width 200 ms +/- 20 --> 1
#define DcfMax1			Ticks(220)

#define DcfState_0		0		// Seen 0
#define DcfState_1		1		// Seen 1
#define DcfState_Sync	2		// Waiting for end-of-minute marker
#define	DcfState_Pon	3		// Power-on interval

unsigned char dcfState;
unsigned char dcfPulse;
unsigned char bitNo;
unsigned leadingTime;

static void DcfInterruptHandler(void);	// Formard

void DcfDecoderInit(task_t *dcfTask)
{
	dcfTask->timer = DcfPonInterval;	// Gives the required startup signal for the DCF module

#if 0	// DCF disabled for the moment

	pinMode(DcfInputPin, INPUT_PULLUP);
	pinMode(DcfPonPin, OUTPUT);

	attachInterrupt(digitalPinToInterrupt(DcfInputPin), DcfInterruptHandler, CHANGE);

	digitalWrite(DcfPonPin, HIGH);		// Drive the pin high (DCF off)
	dcfState = DcfState_Pon;
	dcfPulse = 2;
#endif
}

void DcfDecoder(task_t *dcfTask, unsigned long elapsed)
{
	dcfTask->timer += DcfInterval;

#if 0	// DCF disabled for the moment
	switch (dcfState)
	{
	case DcfState_Pon:
		digitalWrite(DcfPonPin, LOW);
		break;

	case DcfState_Sync:
		break;

	case DcfState_0:
		break;

	case DcfState_1:
		break;

	default:
		break;
	}
#endif
}

static void DcfInterruptHandler(void)
{
	unsigned tim = ReadTime();			// As close as possible to the edge time
	unsigned char pinstate = digitalRead(DcfInputPin);

	setled(seg_ldp1, pinstate==HIGH?1:0);
	display_change |= change_leds;

	if ( dcfState == DcfState_Pon )
	{
		// First edge since power-on
		if ( pinstate == HIGH )
		{
			// A leading edge - record the time and go to sync state.
			leadingTime = tim;
			dcfState = DcfState_Sync;
		}
		return;
	}

	unsigned width = tim - leadingTime;

	if ( width < DcfDebounce )
		return;							// Ignore any changes that come too close together

	if ( dcfState == DcfState_Sync )
	{
		// Looking for a leading-edge to leading-edge of approx. 2 seconds
		if ( pinstate == HIGH )
		{
			leadingTime = tim;
			if ( width >= DcfMinSync && width <= DcfMaxSync )
			{
				// Start receiving bits
				dcfState = DcfState_1;
				bitNo = 0;
			}
			return;
		}
		return;		// Ignore trailing edges during sync search
	}

	if ( dcfState == DcfState_1 && pinstate == LOW )
	{
		// Had a pulse
		if ( width >= DcfMin0 && width <= DcfMax0 )
		{
			// A good 0 pulse - signal the background task
			dcfPulse = 0;
			return;
		}
		if ( width >= DcfMin1 && width <= DcfMax1 )
		{
			// A good 1 pulse
			dcfPulse = 1;
			return;
		}
		// Out-of-spec pulse - try to resync.
		dcfPulse = 2;
		dcfState = DcfState_Sync;
		return;
	}
	if ( dcfState == DcfState_0 && pinstate == HIGH )
	{
		leadingTime = tim;
	}
}

#if 0
unsigned char calcParity(unsigned char x)
{
	unsigned char p = x;
	p = (p ^ (p >> 4));
	p = (p ^ (p >> 2));
	p = (p ^ (p >> 1)) & 0x01;
	return p;
}


// DCF Sample task
#define dcfSampleInterval	5		//	5 milliseconds
#define dcfDebounce			50		//	50 milliseconds
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

// Extracted time and date.
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
	dcfTask->timer = dcfPowerOnInterval;
}

void DcfSample(task_t *dcfTask, unsigned long elapsed)
{
	dcfWidth += dcfSampleInterval;
	if ( dcfState == 0 )
	{
		// Sample the state of the DCF input.
		if ( digitalRead(DcfInputPin) == dcfState1 && dcfWidth > dcfDebounce )
		{
			// Change of state. Start measuring the width of the pulse.
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
			// No leading edge seen --> it's a gap.
			// Leading edge of next pulse is start of a new minute.
			dcfState = 2;
			dcfWidth = 0;
			dcfCalculateTime();
		}
	}
	else
	if ( dcfState == 1 )
	{
		// Sample the state of the DCF input.
		if ( digitalRead(DcfInputPin) == dcfState0 && dcfWidth > dcfDebounce )
		{
			// Change of state. Push the bit value into the shift register.
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
		// Waiting for start of minute.
		if ( digitalRead(DcfInputPin) == dcfState1 )
		{
			// Change of state. Start measuring the width of the pulse.
			unsigned char i;

			dcfState = 1;
			dcfWidth = 0;
			if ( dcfDow != 0 )
			{
				dcfWriteTime();
			}
			dcfBitCount = 0;
			dcfSecond = 0;
			for ( i = 0; i < 8; i++ )
			{
				dcfShiftReg[i] = 0;
			}
			SetDigitBit(4, 3, 1);
			SetDigitBit(4, 4, 1);
		}
	}
	else
	{
		// End of delay to allow DCF to stabilise; turn it on.
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

#endif
