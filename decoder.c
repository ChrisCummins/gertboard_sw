//=============================================================================
//
//
// Decoder: using the bottons as a binary number and turning on one of 8 LEDs
//
// main file
//
// This file is part of the gertboard test suite
//
//
// Copyright (C) Gert Jan van Loo & Myra VanInwegen 2012
// No rights reserved
// You may treat this program as if it was in the public domain
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//

#include "gb_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

//
// Set GPIO pins to the right mode
// Decoder GPIO mapping:
//         Function            Mode
// GPIO0=  LED (D12)           Output
// GPIO1=  LED (D11)           Output
// GPIO4=  LED (D10)           Output
// GPIO7=  LED (D9)            Output
// GPIO8=  LED (D8)            Output
// GPIO9=  LED (D7)            Output
// GPIO10= LED (D6)            Output
// GPIO11= LED (D5)            Output
// GPIO14= unused
// GPIO15= unused
// GPIO17= unused
// GPIO18= unused
// GPIO21= unused
// GPIO22= unused
// GPIO23= Pushbutton (B3)     Input
// GPIO24= Pushbutton (B2)     Input
// GPIO25= Pushbutton (B1)     Input
//
// Always call INP_GPIO(x) first
// as that is how the macros work
void setup_gpio()
{
   // GP23, 24, & 25 are handling the pushbuttons
   INP_GPIO(23);
   INP_GPIO(24);
   INP_GPIO(25);

   // enable pull-up on GPIO 23,24&25, set pull to 2 (code for pull high)
   GPIO_PULL = 2;
   short_wait();
   // setting bits 23, 24 & 25 below means that the GPIO_PULL is applied to
   // GPIO 23, 24, & 25
   GPIO_PULLCLK0 = 0x03800000;
   short_wait();
   GPIO_PULL = 0;
   GPIO_PULLCLK0 = 0;

   // enable the 8 GPIO pins we use for driving the LEDs
   INP_GPIO(0); OUT_GPIO(0);
   INP_GPIO(1); OUT_GPIO(1);
   INP_GPIO(4); OUT_GPIO(4);
   INP_GPIO(7); OUT_GPIO(7);
   INP_GPIO(8); OUT_GPIO(8);
   INP_GPIO(9); OUT_GPIO(9);
   INP_GPIO(10); OUT_GPIO(10);
   INP_GPIO(11); OUT_GPIO(11);
} // setup_gpio

// the led array allows us to select the LED to turn on; for example led[0] is
// the bit pattern we need to turn on D5, the leftmost LED of the ones
// we are using (and hence it is the output for 0)
static int led[] = {1<<11, 1<<10, 1 <<9, 1<<8, 1<<7, 1<<4, 1<<1, 1};

// remove pulling on pins so they can be used for somnething else next time
// gertboard is used
void unpull_pins()
{
   // disable pull-up on GPIO 23,24&25, set pull to 0 (code for no pull)
   GPIO_PULL = 0;
   short_wait();
   // setting bits 23, 24 & 25 below means that the GPIO_PULL is applied to
   // GPIO 23, 24, & 25
   GPIO_PULLCLK0 = 0x03800000;
   short_wait();
   GPIO_PULL = 0;
   GPIO_PULLCLK0 = 0;
} // unpull_pins

int main(void)
{ int r,d;
  unsigned int b,prev_b;
  char str [4];

  printf ("These are the connections for the buttons test:\n");
  printf ("GP25 in J2 --- B1 in J3\n");
  printf ("GP24 in J2 --- B2 in J3\n");
  printf ("GP23 in J2 --- B3 in J3\n");
  printf ("GP11 in J2 --- B5 in J3\n");
  printf ("GP10 in J2 --- B6 in J3\n");
  printf ("GP9 in J2 --- B7 in J3\n");
  printf ("GP8 in J2 --- B8 in J3\n");
  printf ("GP7 in J2 --- B9 in J3\n");
  printf ("GP4 in J2 --- B10 in J3\n");
  printf ("GP1 in J2 --- B11 in J3\n");
  printf ("GP0 in J2 --- B12 in J3\n");
  printf ("jumper on U4-out-B5\n");
  printf ("jumper on U4-out-B6\n");
  printf ("jumper on U4-out-B7\n");
  printf ("jumper on U4-out-B8\n");
  printf ("jumper on U5-out-B9\n");
  printf ("jumper on U5-out-B10\n");
  printf ("jumper on U5-out-B11\n");
  printf ("jumper on U5-out-B12\n");
  printf ("When ready hit enter.\n");
  (void) getchar();

   // Map the I/O sections
   setup_io();

   // Set GPIO pins 23, 24, and 25 to the required mode
   setup_gpio();

   // read the switches a number of times and light up a different LED
   // to show the result

   /* below we set prev_b to a 0 as it will most likely be different from
      the first number we read from the buttons (user would have to have
      all buttons pressed to get this value) */
   prev_b = 0; 
   
   r = 40; // number of repeats

  while (r)
  {
    b = GPIO_IN0;
    b = (b >> 23 ) & 0x07; // keep only bits 23, 24 & 25
    if (b^prev_b)
    { // one or more buttons changed
      GPIO_CLR0 = led[prev_b];  // turn off LED for prev button setup
      GPIO_SET0 = led[b];  // turn on LED for this setup
      prev_b = b;
      r--;
    } // change
  } // while

  // turn off all LEDs
  for (b = 0; b < 8; b++)
    GPIO_CLR0 = led[b]; 
  // disable pull up on pins & unmap gpio
  unpull_pins();
  restore_io();

  return 0;
} // main

