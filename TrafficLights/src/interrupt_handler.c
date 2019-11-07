/*
 * interrupt_handler.c
 *
 *  Created on: 31 Oct 2019
 *      Author: s1758499
 */
#include "xil_types.h"
#include "seg7_display.h"
#include "gpio_init.h"

u16 interruptCounter = 0;
extern int interruptServiced;
extern int state_1;
extern int state_2;

extern u16 colour_0;
extern u16 colour_1;
extern u16 colour_2;

extern u16 colour_6;
extern u16 colour_7;
extern u16 colour_8;

extern u16 pd_colour;

extern u16 led_out;
extern u16 pd_button;
volatile u16 disp_number = 2;

int waiting = 0;
int tr1_done = 0;
int tr2_done = 1;

int count = 0;

void decodeColours()
{
	switch (state_1)
	{
		case 0:
			colour_0 = 0xF00;
			colour_1 = 0xFFF;
			colour_2 = 0xFFF;
			break;
		case 1:
			colour_0 = 0xF00;
			colour_1 = 0xFF0;
			colour_2 = 0xFFF;
			break;
		case 2:
			colour_0 = 0xFFF;
			colour_1 = 0xFFF;
			colour_2 = 0x0F0;
			break;
		case 3:
			colour_0 = 0xFFF;
			colour_1 = 0xFF0;
			colour_2 = 0xFFF;
	}

	switch (state_2)
	{
		case 0:
			colour_6 = 0xF00;
			colour_7 = 0xFFF;
			colour_8 = 0xFFF;
			break;
		case 1:
			colour_6 = 0xF00;
			colour_7 = 0xFF0;
			colour_8 = 0xFFF;
			break;
		case 2:
			colour_6 = 0xFFF;
			colour_7 = 0xFFF;
			colour_8 = 0x0F0;
			break;
		case 3:
			colour_6 = 0xFFF;
			colour_7 = 0xFF0;
			colour_8 = 0xFFF;
	}
}

void decodeLeds()
{
	if (state_1 == 0 && state_2 == 0) led_out = 0x8004;
	else if (state_1 == 1) led_out = 0xC004;
	else if (state_1 == 2) led_out = 0x2004;
	else if (state_1 == 3) led_out = 0x4004;
	else if (state_2 == 1) led_out = 0x8006;
	else if (state_2 == 2) led_out = 0x8001;
	else if (state_2 == 3) led_out = 0x8002;
}

void hwTimerISR(void *CallbackRef)
{
	interruptServiced = FALSE;

	displayDigit();

	if (pd_button || waiting)
	{
		waiting = 1;

		if (tr1_done)
		{
			count++;

			pd_colour = 0x0F0;
			if (count == 1250)
			{
				waiting = 0;
				pd_colour = 0xF00;
				count = 0;
			}

			interruptServiced = TRUE;
			return;
		}
	}

	interruptCounter++;

	if (interruptCounter == 250)
		disp_number = 1;

	if (interruptCounter == 500)
	{
		disp_number = 2;
		if (tr2_done)
		{
			state_1 = (state_1 + 1) % 4;
			tr1_done = state_1 == 0 ? 1 : 0;
			tr2_done = !tr1_done;
		}

		else if (tr1_done)
		{
			state_2 = (state_2 + 1) % 4;
			tr2_done = state_2 == 0 ? 1 : 0;
			tr1_done = !tr2_done;
		}

		interruptCounter = 0;
		decodeColours();
		decodeLeds();
	}

	interruptServiced = TRUE;
}