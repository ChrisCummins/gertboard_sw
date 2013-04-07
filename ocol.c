//
//
// Gertboard test suite
//
// These program works the open collector
//
//
// This file is part of gertboard test suite.
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


// open colloector test GPIO mapping:
//         Function            Mode
// GPIO0=  unused
// GPIO1=  unused
// GPIO4=  open collector      Output
// GPIO7=  unused
// GPIO8=  unused
// GPIO9=  unused
// GPIO10= unused
// GPIO11= unused
// GPIO14= unused (preset to be UART)
// GPIO15= unused (preset to be UART)
// GPIO17= unused
// GPIO18= unused
// GPIO21= unused
// GPIO22= unused
// GPIO23= unused
// GPIO24= unused
// GPIO25= unused

void setup_gpio(void)
{
  INP_GPIO(4);  OUT_GPIO(4);
} // setup_gpio


//
// send on/off signals to GPIO4 - it's the wiring on and off the board
// that makes interesting things happen
//
int main(void)
{ int p,r,last, chan;

  do {
    printf ("Which driver do you want to test?\n");
    printf ("Type a number between 1 and 6.\n");
    chan  = (int) getchar();
    (void) getchar(); // eat carriage return
  } while (chan < '1' | chan > '6');
  chan = chan - '0';

  printf ("These are the connections for the open collector test:\n");
  printf ("GP4 in J2 --- RLY%d in J4\n", chan);
  printf ("+ of external power source --- RPWR in J6\n");
  printf ("ground of external power source --- GND (any)\n");
  printf ("ground side of your circuit --- RLY%d in J%d\n", chan, chan + 11);
  printf ("When ready hit enter.\n");
  (void) getchar();

  // Map the I/O sections
  setup_io();

  // Set GPIO4 pin to output mode
  setup_gpio();

  for (p = 0; p < 10; p++) {
    GPIO_SET0 = 1 << 4;
    long_wait(10);
    GPIO_CLR0 = 1 << 4;
    long_wait(10);
  }

  restore_io();
} // main



