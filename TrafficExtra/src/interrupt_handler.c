/*
 * interrupt_handler.c
 *
 *  Created on: 31 Oct 2019
 *      Author: s1758499
 */
#include "xil_types.h"
#include "seg7_display.h"
#include "gpio_init.h"
#include "interrupt.h"
#include "colours.h"

// Variable that will hold the number of interrupts, in order to determine when a second has passed etc.
u16 interruptCounter = 0;

// Initial value of the number to be displayed on the 7-segment
volatile u16 disp_number = 3;

// Boolean flags
int waiting = 0; // Is pedestrian waiting?
int crossing = 0; // Is pedestrian crossing?
int tr1_done = 0; // Has TR1 finished its sequence?
int tr2_done = 1; // Has TR2 finished its sequence?
int just_crossed = 0; // Was a pedestrian just allowed to cross?

int mult = 1;

// Time counter for when a pedestrian is going to cross
u16 count = 0;

int bigger = 0;

void decodeColours()
{
	// Based on state of TR1, set the values of colour_{0,3}
	switch (state_1)
	{
		// RED
		case 0:
			colour_0 = RED;
			colour_1 = WHITE;
			colour_2 = WHITE;
			break;
		// RED + YELLOW
		case 1:
			colour_0 = RED;
			colour_1 = YELLOW;
			colour_2 = WHITE;
			break;
		// GREEN
		case 2:
			colour_0 = WHITE;
			colour_1 = WHITE;
			colour_2 = GREEN;
			break;
		// YELLOW
		case 3:
			colour_0 = WHITE;
			colour_1 = YELLOW;
			colour_2 = WHITE;
	}

	// Based on state of TR2, set the values of colour_{6,8}
	switch (state_2)
	{
		case 0:
			colour_6 = RED;
			colour_7 = WHITE;
			colour_8 = WHITE;
			break;
		case 1:
			colour_6 = RED;
			colour_7 = YELLOW;
			colour_8 = WHITE;
			break;
		case 2:
			colour_6 = WHITE;
			colour_7 = WHITE;
			colour_8 = GREEN;
			break;
		case 3:
			colour_6 = WHITE;
			colour_7 = YELLOW;
			colour_8 = WHITE;
	}
}

void decodeLeds()
{
	// Produce the LED value by checking the states of both traffic lights
	if (state_1 == 0 && state_2 == 0) led_out = 0x8004;
	else if (state_1 == 1) led_out = 0xC004;
	else if (state_1 == 2) led_out = 0x2004;
	else if (state_1 == 3) led_out = 0x4004;
	else if (state_2 == 1) led_out = 0x8006;
	else if (state_2 == 2) led_out = 0x8001;
	else if (state_2 == 3) led_out = 0x8002;
}

void blink()
{
	// If the counter is within one of the five ranges, change to colour to white
	// This will give a blinking effect
	if ((count >= 1500 && count < 1550) ||
		(count >= 1600 && count < 1650) ||
		(count >= 1700 && count < 1750) ||
		(count >= 1800 && count < 1850) ||
		(count >= 1900 && count < 1950)
	) pd_colour = 0xFFF;
}

int enablePedestrian()
{
	// Check if the pedestrian is still waiting
	waiting = !crossing;

	// Light up the 8-th LED if he's waiting
	led_out = led_out ^ waiting << 8;

	// If both lights are red, allow him to cross (after 3 seconds)
	if (state_1 == 0 && state_2 == 0 && !just_crossed)
	{
		// Increment the crossing time counter
		count++;

		// Decrement the number showing when he'll be able to cross
		disp_number = 3 - count / 250;

		// After 3 seconds of both lights being red, turn PD light green
		if (count >= 750 && count < 2000)
		{
			// Indicate that the PD is not waiting anymore
			waiting = 0;
			crossing = 1; // He is crossing now
			pd_colour = 0x0F0; // Light is green
			blink(); // Blink (if 2 seconds are left)
			disp_number = 5 - (count - 750) / 250; // Decrement the time left for crossing
		}

		// If the PD crossing time is over
		else if (count == 2000)
		{
			crossing = 0; // Not crossing anymore
			pd_colour = 0xF00;
			count = 0;
			disp_number = 3;
			just_crossed = 1;
		}

		// Return 1 if pedestrian is allowed to cross now
		return 1;
	}

	return 0;
}

void updateTR1()
{
	// If the traffic at TR1 is bigger, give longer green light to it
	if (state_1 == 2 && bigger == 1)
	{
		if (interruptCounter == 750 * mult)
		{
			state_1 = 3;
			interruptCounter = 0;
		}
	}

	// If the traffics are the same or the light is not green, display it for 3 seconds
	else
	{
		if (interruptCounter == 750)
		{
			state_1 = (state_1 + 1) % 4; // Go to next state
			interruptCounter = 0;
			tr1_done = state_1 == 0 ? 1 : 0; // Check if sequence is finished
			tr2_done = !tr1_done; // If first light is finished, make light 2 start
		}
	}
}

// Similar to updateTR1()
void updateTR2()
{
	if (state_2 == 2 && bigger == 2)
	{
		if (interruptCounter == 750 * mult)
		{
			state_2 = 3;
			interruptCounter = 0;
		}
	}

	else
	{
		if (interruptCounter == 750)
		{
			state_2 = (state_2 + 1) % 4;
			interruptCounter = 0;
			tr2_done = state_2 == 0 ? 1 : 0;
			tr1_done = !tr2_done;
		}
	}
}

void updateStates()
{
	// Update the lights if one of them has finished its sequence
	if (tr2_done) updateTR1();
	else if (tr1_done) updateTR2();

	// Decode the new colours and LEDs if the counter has been reset
	if (!interruptCounter)
	{
		decodeColours();
		decodeLeds();
		just_crossed = 0;
	}
}

void getMultiplier()
{
	// If traffic_2 is zero and traffic_1 non-zero, give 3 times longer green light to TR1
	if (traffic_1 > 0 && traffic_2 == 0)
	{
		mult = 3;
		bigger = 1;
	}

	// Or vice versa
	else if (traffic_1 == 0 && traffic_2 > 0)
	{
		mult = 3;
		bigger = 2;
	}

	// If both non-zero, give n-times more green light time to the road with n-times more traffic
	else if (traffic_1 > traffic_2)
	{
		mult = traffic_1 / traffic_2;
		bigger = 1;
	}

	else if (traffic_1 < traffic_2)
	{
		mult = traffic_2 / traffic_1;
		bigger = 2;
	}
}

void hwTimerISR(void *CallbackRef)
{
	interruptServiced = FALSE;

	// Display the current value of disp_number
	displayDigit();

	// Check if the pedestrian button has been pressed or the pedestrian is waiting or crossing
	if (pd_button || waiting || crossing)
	{
		// If pedestrian is crossing, finish the interrupt handling
		if (enablePedestrian())
		{
			interruptServiced = TRUE;
			return;
		}
	}

	interruptCounter++;
	getMultiplier();

	// Reduce the timer value
	disp_number = (state_1 == 2 && bigger == 1) || (state_2 == 2 && bigger == 2) ?
			3 * mult - (interruptCounter) / 250 : 3 - (interruptCounter) / 250;

	// Update state of the lights after 3 seconds
	// (this function will only do something after at least 3 seconds have passed)
	updateStates();

	interruptServiced = TRUE;
}
