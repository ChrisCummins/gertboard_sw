//
// Gertboard Demo
//
// SPI (ADC/DAC) control code
//
// This code is part of the Gertboard test suite
// These routines access the AD and DA chips
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
// Try to strike a balance between keep code simple for
// novice programmers but still have reasonable quality code
//

#include "gb_common.h"
#include "gb_spi.h"

// Set GPIO pins to the right mode
// dad (digital-analogue-digital) GPIO mapping:
//         Function            Mode
// GPIO0=  unused
// GPIO1=  unused
// GPIO4=  unused
// GPIO7=  SPI chip select B   Alt. 0
// GPIO8=  SPI chip select A   Alt. 0
// GPIO9=  SPI MISO            Alt. 0
// GPIO10= SPI MOSI            Alt. 0
// GPIO11= SPI CLK             Alt. 0
// GPIO14= unused
// GPIO15= unused
// GPIO17= unused
// GPIO18= unused
// GPIO21= unused
// GPIO22= unused
// GPIO23= unused
// GPIO24= unused
// GPIO25= unused
//

// For A to D  and D to A we need the SPI bus and SPI chip selects A & B
void setup_gpio()
{
   INP_GPIO(7);  SET_GPIO_ALT(7,0);
   INP_GPIO(8);  SET_GPIO_ALT(8,0);
   INP_GPIO(9);  SET_GPIO_ALT(9,0);
   INP_GPIO(10); SET_GPIO_ALT(10,0);
   INP_GPIO(11); SET_GPIO_ALT(11,0);
} // setup_gpio


//
//  Do digital to analogue to digital conversion
//
void main(void)
{ int d, dac_val, v, s, i;

  printf ("These are the connections for the digital to analogue to digital test:\n");
  printf ("jumper connecting GP11 to SCLK\n");
  printf ("jumper connecting GP10 to MOSI\n");
  printf ("jumper connecting GP9 to MISO\n");
  printf ("jumper connecting GP8 to CSnA\n");
  printf ("jumper connecting GP7 to CSnB\n");
  printf ("jumper connecting DA1 on J29 to AD0 on J28\n");
  printf ("When ready hit enter.\n");
  (void) getchar();

  // Map the I/O sections
  setup_io();

  // activate SPI bus pins
  setup_gpio();

  // Setup SPI bus
  setup_spi();

  // The value returned by the A to D can jump around quite a bit, so 
  // simply printing out the value isn't very useful. The bar graph
  // is better because this hides the noise in the signal.

  printf ("dig ana\n");
  for (d=0; d <= 256; d+= 32)
  {
    if (d == 256) 
      dac_val = 255 * 16;
    else 
      dac_val = d * 16;
    write_dac(1, dac_val);
    v= read_adc(0);
    // v should be in range 0-1023
    // map to 0-63
    s = v >> 4;
    printf("%3x %04d ", dac_val, v);
    // show horizontal bar
    for (i = 0; i < s; i++)
      putchar('#');
    for (i = 0; i < 64 - s; i++)
      putchar(' ');
    putchar('\n');
    short_wait();
  } // repeated write/read
  for (d=224; d >= 0; d-= 32)
  {
    dac_val = d * 16;
    write_dac(1, dac_val);
    v= read_adc(0);
    // v should be in range 0-1023
    // map to 0-63
    s = v >> 4;
    printf("%3x %04d ", dac_val, v);
    // show horizontal bar
    for (i = 0; i < s; i++)
      putchar('#');
    for (i = 0; i < 64 - s; i++)
      putchar(' ');
    putchar('\n');
    short_wait();
  } // repeated write/read

  printf("\n");
  restore_io();
} // main
