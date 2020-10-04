//  Auther: Derek Cooper
//  Based on the work of and credit given to:
//  Fred.Chu
//  Detlef Giessmann Germany
//
//  Date:20 September 2020
//
//  Applicable Module:
//                     Gotek 3-digit LED display driver
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//Modified record:
//Autor:        Detlef Giessmann Germany
//Mail: derek_cooper@hotmail.com
//Library to drive LEDC68 Gotek 3 digit LED display
//IDE:          Arduino-1.8.12
/***************************************************************/
//
#include "TM1651.h"
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>

static int8_t NumTab[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f,
		0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00, 0x63, 0x5c, 0x01, 0x40,
		0x08 }; //numbers 0-9, A-F, special chars

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("not enough parameters");
		return 1;
	}
	char *text = argv[1];
	int dp = atoi(argv[2]);
	printf("DP: %d\n", dp);
	printf("text to display: %s\n", text);
	printf("about to set up WiringPi\n");
	if (wiringPiSetup() == -1)
		return 1;
	printf("about to construct\n");
	TM1651 display(9, 8);
	printf("about to display clear\n");
	display.displayClear();
	printf("about to display set\n");
	display.displaySet(4);
	printf("about to display characters\n");
	display.displayDP(dp);
	int textPos = 0;
	int displayPos = 0;
	while (text[textPos] > 0 && displayPos < 3) {
		char character = text[textPos];
		if (character == '.') {
//			display.displayDP(dp);
		} else {
			display.displayCharacter(displayPos, character);
			displayPos++;
		}
		textPos++;
	}
	while (displayPos < 3) {
		display.displayCharacter(displayPos, ' ');
		displayPos++;
	}
}

TM1651::TM1651(uint8_t Clk, uint8_t Data) {
	Clkpin = Clk;
	Datapin = Data;
	pinMode(Clkpin, OUTPUT);
	pinMode(Datapin, OUTPUT);
}

void TM1651::writeByte(int8_t wr_data) {
	uint8_t i, count1 = 0;
	for (i = 0; i < 8; i++) {				// send 8bit data
		digitalWrite(Clkpin, LOW);
		if (wr_data & 0x01)
			digitalWrite(Datapin, HIGH);	// LSB first
		else
			digitalWrite(Datapin, LOW);
		delayMicroseconds(COUNT / 2);
		wr_data >>= 1;
		digitalWrite(Clkpin, HIGH);
		delayMicroseconds(COUNT / 2);
	}
	digitalWrite(Clkpin, LOW);				// wait for the ACK
	digitalWrite(Datapin, HIGH);
	delayMicroseconds(COUNT / 4);
	digitalWrite(Clkpin, HIGH);
	pinMode(Datapin, INPUT);
	while (digitalRead(Datapin)) {
		count1 += 1;
		if (count1 == 200) {
			pinMode(Datapin, OUTPUT);
			digitalWrite(Datapin, LOW);
			count1 = 0;
		}
		pinMode(Datapin, INPUT);
	}
	pinMode(Datapin, OUTPUT);
}

//send start signal to TM1651
void TM1651::start(void) {
	digitalWrite(Clkpin, HIGH);				// send start signal to TM1651
	digitalWrite(Datapin, HIGH);
	delayMicroseconds(COUNT / 2);
	digitalWrite(Datapin, LOW);
	delayMicroseconds(COUNT / 2);
	digitalWrite(Clkpin, LOW);
}
//End signal
void TM1651::stop(void) {
	digitalWrite(Clkpin, LOW);
	digitalWrite(Datapin, LOW);
	delayMicroseconds(COUNT / 2);
	digitalWrite(Clkpin, HIGH);
	delayMicroseconds(COUNT / 2);
	digitalWrite(Datapin, HIGH);
}

//******************************************
void TM1651::displayNum(uint8_t dig, uint8_t number) {
	start();								// start signal sent to TM1651 from MCU
	writeByte(ADDR_FIXED);
	stop();
	start();
	writeByte(STARTADDR + dig);				// digit pos 0-2
	writeByte(NumTab[number]);
	stop();
	start();
	writeByte(Cmd_DispCtrl); 				// 88+0 to 7 brightness, 88=display on
	stop();
}

//******************************************
void TM1651::displayRaw(uint8_t dig, uint8_t number) {
	printf("displayRaw: %d %d\n", dig, number);
	start();								// start signal sent to TM1651 from MCU
	writeByte(ADDR_FIXED);
	stop();
	start();
	writeByte(STARTADDR + dig);				// digit pos 0-2
	writeByte(number);
	stop();
	start();
	writeByte(Cmd_DispCtrl);				// 88+0 to 7 brightness, 88=display on
	stop();
}

//******************************************
void TM1651::displayCharacter(uint8_t dig, char character) {
	printf("displayCharacter: %d %d\n", dig, character);
	start();								// start signal sent to TM1651 from MCU
	writeByte(ADDR_FIXED);
	stop();
	start();
	writeByte(STARTADDR + dig);				// digit pos 0-2
	writeByte(getCharacterCode(character));
	stop();
	start();
	writeByte(Cmd_DispCtrl);				// 88+0 to 7 brightness, 88=display on
	stop();
}

//******************************************
void TM1651::displayInteger(uint16_t number) {
	uint8_t i;

	if (number > 999)
		number = 999;
	start();								//start signal sent to TM1651 from MCU
	writeByte(ADDR_AUTO);					// auto increment the address
	stop();
	start();
	writeByte(STARTADDR);					// start at 0
	writeByte(NumTab[(number / 100) % 10]);
	writeByte(NumTab[(number / 10) % 10]);
	writeByte(NumTab[number % 10]);
	stop();
	start();
	writeByte(Cmd_DispCtrl);				// 88+0 to 7 brightness, 88=display on
	stop();
}

//******************************************
void TM1651::displayDP(uint8_t dp) {
	uint8_t SegData;

	//if (dp == 1) SegData = 0x08;
	//else SegData = 0x00;
	start();								// start signal sent to TM1651 from MCU
	writeByte(ADDR_FIXED);
	stop();
	start();
	writeByte(STARTADDR + 4);				// digit pos 3 controls decimal point
	//writeByte(SegData);
	writeByte(dp);
	stop();
	start();
	writeByte(Cmd_DispCtrl);				// 88+0 to 7 brightness, 88=display on
	stop();
}

void TM1651::displayClear(void) {
	displayDP(0);
}

void TM1651::displaySet(uint8_t brightness) {
	Cmd_DispCtrl = 0x88 + brightness;		// Set the brightness and turn on
}

void TM1651::displayOff() {
	Cmd_DispCtrl = 0x80;
	start();
	writeByte(Cmd_DispCtrl);				// 88+0 to 7 brightness, 88=display on
	stop();
}

uint8_t TM1651::getCharacterCode(char character) {
	switch (character) {
		case '0':
			return 0x3f;
		case '1':
			return 0x06;
		case '2':
			return 0x5b;
		case '3':
			return 0x4f;
		case '4':
			return 0x66;
		case '5':
			return 0x6d;
		case '6':
			return 0x7c;
		case '7':
			return 0x07;
		case '8':
			return 0x7f;
		case '9':
			return 0x6f;
		case 'A':
			return 119;
		case 'a':
			return 95;
		case 'B':
		case 'b':
			return 124;
		case 'C':
			return 57;
		case 'c':
			return 88;
		case 'D':
		case 'd':
			return 94;
		case 'E':
		case 'e':
			return 121;
		case 'F':
		case 'f':
			return 113;
		case 'G':
		case 'g':
			return 61;
		case 'H':
			return 118;
		case 'h':
			return 116;
		case 'I':
		case 'i':
			return 6;
		case 'J':
		case 'j':
			return 30;
		case 'K':
		case 'k':
			return 122;
		case 'L':
			return 56;
		case 'l':
			return 24;
		case 'N':
			return 55;
		case 'n':
			return 84;
		case 'O':
			return 63;
		case 'o':
			return 92;
		case 'P':
		case 'p':
			return 115;
		case 'Q':
		case 'q':
			return 103;
		case 'R':
			return 49;
		case 'r':
			return 80;
		case 'S':
		case 's':
			return 109;
		case 'T':
		case 't':
			return 120;
		case 'U':
			return 62;
		case 'u':
			return 28;
		case 'Y':
		case 'y':
			return 110;
		case 'Z':
		case 'z':
			return 0x5b;
		default:
			return 0x00;
	}
}
