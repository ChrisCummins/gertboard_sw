//=============================================================================
//
//
// Buttons_test
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
// button test GPIO mapping:
//         Function            Mode
// GPIO0=  unused
// GPIO1=  unused
// GPIO4=  unused
// GPIO7=  unused
// GPIO8=  unused
// GPIO9=  unused
// GPIO10= unused
// GPIO11= unused
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
   // for this test we are only using GP23, 24, & 25
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
} // setup_gpio

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
  printf ("Optionally, if you want the LEDs to reflect button state do the following:\n");
  printf ("jumper on U3-out-B1\n");
  printf ("jumper on U3-out-B2\n");
  printf ("jumper on U3-out-B3\n");
  printf ("When ready hit enter.\n");
  (void) getchar();

   // Map the I/O sections
   setup_io();

   // Set GPIO pins 23, 24, and 25 to the required mode
   setup_gpio();

   // read the switches a number of times and print out the result

   /* below we set prev_b to a number which will definitely be different
      from what the switches returns, as after shift & mask, b can only
      be in range 0..7 */
   prev_b = 8; 
   
   r = 20; // number of repeats

  while (r)
  {
    b = GPIO_IN0;
    b = (b >> 23 ) & 0x07; // keep only bits 23, 24 & 25
    if (b^prev_b)
    { // one or more buttons changed
      make_binary_string(3, b, str);
      printf("%s\n", str);
      prev_b = b;
      r--;
    } // change
  } // while

  // disable pull up on pins & unmap gpio
  unpull_pins();
  restore_io();

  return 0;
} // main

