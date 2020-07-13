# dcfclock

## A retro-style digital clock using a red 7-segment LED display

Features

* Four digit display with flashing colon
* Infra-red remote control for setting
* DCF77 synchronizaton

## The DCF77 signal.

See https://en.wikipedia.org/wiki/DCF77 for details

The time and date is transmitted at a rate of one bit per second. A narrow pulse (100 ms) is a 0,
a wide pulse (200 ms) is a 1

| Byte no. | Bit no. | DCF bit | Weight | Description                                                   |
|:---------|:--------|:--------|:-------|:--------------------------------------------------------------|
| 0        | 0       | :00     | M      | Start of minute. Always 0.                                    |
| 0        | 1       | :01     | ?      | Civil warning and weather bits                                |
| 0        | 2       | :02     | ?      |                                                               |
| 0        | 3       | :03     | ?      |                                                               |
| 0        | 4       | :04     | ?      |                                                               |
| 0        | 5       | :05     | ?      |                                                               |
| 0        | 6       | :06     | ?      |                                                               |
| 0        | 7       | :07     | ?      |                                                               |
| 1        | 0       | :08     | ?      |                                                               |
| 1        | 1       | :09     | ?      |                                                               |
| 1        | 2       | :10     | ?      |                                                               |
| 1        | 3       | :11     | ?      |                                                               |
| 1        | 4       | :12     | ?      |                                                               |
| 1        | 5       | :13     | ?      |                                                               |
| 1        | 6       | :14     | ?      |                                                               |
| 1        | 7       | :15     | R      | Call bit: abnormal transmitter operation.                     |
| 2        | 0       | :16     | A1     | Summer time announcement. Set during hour before change.      |
| 2        | 1       | :17     | Z1     | Set to 1 when CEST is in effect.                              |
| 2        | 2       | :18     | Z2     | Set to 1 when CET is in effect.                               |
| 2        | 3       | :19     | A2     | Leap second announcement. Set during hour before leap second. |
| 2        | 4       | :20     | S      | Start of encoded time. Always 1.                              |
| 2        | 5       | :21     | 1      | Minutes                                                       |
| 2        | 6       | :22     | 2      |                                                               |
| 2        | 7       | :23     | 4      |                                                               |
| 3        | 0       | :24     | 8      |                                                               |
| 3        | 1       | :25     | 10     |                                                               |
| 3        | 2       | :26     | 20     |                                                               |
| 3        | 3       | :27     | 40     |                                                               |
| 3        | 4       | :28     | P1     | Even parity over minute bits                                  |
| 3        | 5       | :29     | 1      | Hours                                                         |
| 3        | 6       | :30     | 2      |                                                               |
| 3        | 7       | :31     | 4      |                                                               |
| 4        | 0       | :32     | 8      |                                                               |
| 4        | 1       | :33     | 10     |                                                               |
| 4        | 2       | :34     | 20     |                                                               |
| 4        | 3       | :35     | P2     | Even parity over hour bits                                    |
| 4        | 4       | :36     | 1      | Day of month.                                                 |
| 4        | 5       | :37     | 2      |                                                               |
| 4        | 6       | :38     | 4      |                                                               |
| 4        | 7       | :39     | 8      |                                                               |
| 5        | 0       | :40     | 10     |                                                               |
| 5        | 1       | :41     | 20     |                                                               |
| 5        | 2       | :42     | 1      | Day of week Monday=1, Sunday=7                                |
| 5        | 3       | :43     | 2      |                                                               |
| 5        | 4       | :44     | 4      |                                                               |
| 5        | 5       | :45     | 1      | Month number                                                  |
| 5        | 6       | :46     | 2      |                                                               |
| 5        | 7       | :47     | 4      |                                                               |
| 6        | 0       | :48     | 8      |                                                               |
| 6        | 1       | :49     | 10     |                                                               |
| 6        | 2       | :50     | 1      | Year within century                                           |
| 6        | 3       | :51     | 2      |                                                               |
| 6        | 4       | :52     | 4      |                                                               |
| 6        | 5       | :53     | 8      |                                                               |
| 6        | 6       | :54     | 10     |                                                               |
| 6        | 7       | :55     | 20     |                                                               |
| 7        | 0       | :56     | 40     |                                                               |
| 7        | 1       | :57     | 80     |                                                               |
| 7        | 2       | :58     | P3     | Even parity over date bits                                    |
| 7        | 3       | :59     | -      | Minute mark: no amplitude modulation                          |

## Caveat

The original version (before the existence of this file) used a 4-digit multiplexed display.

The revised version uses individual 7-segment displays driven by shift registers that
are controlled using SPI. The shift registers have open-collector outputs so might be 74LS596 or 74LS599
devices. You could probably use 74HC595 devices instead.

## License

(c) David Haworth

dcfclock is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

dcfclock is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dcfclock.  If not, see <http://www.gnu.org/licenses/>.
