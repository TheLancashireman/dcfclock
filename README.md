# dcfclock

## A retro-style digital clock using a red 7-segment LED display

Features

* Four digit display with flashing colon
* Infra-red remote control for setting
* DCF77 synchronizaton

## Caveat

The original version (before the existence of this file) used a 4-digit multiplexed display.

The revised version uses individual 7-segment displays driven by 74LS596 shift registers that
are controlled using SPI.

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
