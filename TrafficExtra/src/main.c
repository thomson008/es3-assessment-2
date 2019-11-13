/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "gpio_init.h"
#include "seg7_display.h"
#include "main.h"


// Number to be displayed on the screen
extern u16 disp_number;

// Boolean flag denoting whether an interrupt has been serviced
volatile int interruptServiced = FALSE;

// Methods declarations
void print(char *str);
XStatus initGpio();
XStatus setUpInterruptSystem();

// Method to keep writing and reading values to all the devices
void controlXGpios()
{
	while (1)
	{
		// Write colours to the first traffic light
		XGpio_DiscreteWrite(&REGION_0_COLOUR, 1, colour_0);
		XGpio_DiscreteWrite(&REGION_1_COLOUR, 1, colour_1);
		XGpio_DiscreteWrite(&REGION_2_COLOUR, 1, colour_2);

		// Write colours to the second traffic light
		XGpio_DiscreteWrite(&REGION_6_COLOUR, 1, colour_6);
		XGpio_DiscreteWrite(&REGION_7_COLOUR, 1, colour_7);
		XGpio_DiscreteWrite(&REGION_8_COLOUR, 1, colour_8);

		// Write colour to the pedestrian light
		XGpio_DiscreteWrite(&REGION_4_COLOUR, 1, pd_colour);

		// Write value to LEDs
		XGpio_DiscreteWrite(&LEDs, 1, led_out);

		// Read the value from the pedestrian button
		pd_button = XGpio_DiscreteRead(&PD_BTN, 1);

		// Display some number (may be time left for a traffic light or time left for pedestrian)
		displayNumber(disp_number);

		// Read traffic amounts
		slideSwitchIn = XGpio_DiscreteRead(&SWITCHES, 1);
		traffic_1 = slideSwitchIn >> 8;
		traffic_2 = slideSwitchIn & 0xFF;

		// Wait for the interrupt to be serviced
		while (!interruptServiced);
	}
}

int main()
{
	init_platform();

	// Initialize all the Gpios and check if everything worked correctly
	XStatus status = initGpio();
	if (status != XST_SUCCESS)
	{
		print("GPIOs initialisation failed!!!\n\r");
		cleanup_platform();
		return 0;
	}

	print("GPIOs successfully initialised.\n\r");

	// Set up the interrupt system and perform a similar check as before
	status = setUpInterruptSystem();
	if (status != XST_SUCCESS)
	{
		print("Something wrong with interrupt!\n\r");
		cleanup_platform();
		return 0;
	}

	print("Interrupt successfully set.\n\r");

	// Call the function that will keep reading and writing Gpios values forever
	controlXGpios();

	cleanup_platform();
	return 0;
}


