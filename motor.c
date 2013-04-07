//=============================================================================
//
//
// Gertboard tests
//
// This code is part of the Gertboard test suite
// motor control part
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
#include "gb_pwm.h"

// motor test GPIO mapping:
//         Function            Mode
// GPIO0=  unused
// GPIO1=  unused
// GPIO4=  unused
// GPIO7=  unused
// GPIO8=  unused
// GPIO9=  unused
// GPIO10= unused
// GPIO11= unused
// GPIO14= unused (preset to be UART)
// GPIO15= unused (preset to be UART)
// GPIO17= motor control B     Output
// GPIO18= PWM (motor A)       alt ftn 5
// GPIO21= unused
// GPIO22= unused
// GPIO23= unused
// GPIO24= unused
// GPIO25= unused

void setup_gpio(void)
{
  INP_GPIO(17);  OUT_GPIO(17);
  INP_GPIO(18);  SET_GPIO_ALT(18, 5);
} // setup_gpio


void main()
{ int r,s;

  printf ("These are the connections for the motor test:\n");
  printf ("GP17 in J2 --- MOTB (just above GP1)\n");
  printf ("GP18 in J2 --- MOTA (just above GP4)\n");
  printf ("+ of external power source --- MOT+ in J19\n");
  printf ("ground of external power source --- GND (any)\n");
  printf ("one wire for your motor in MOTA in J19\n");
  printf ("the other wire for your motor in MOTB in J19\n");
  printf ("When ready hit enter.\n");
  (void) getchar();

  // Map the I/O sections
  setup_io();

  // Set GPIO pin 18 to use PWM and pin 17 to output mode
  setup_gpio();

  // set pin controlling the non-PWM driver to low and get PWM ready
  GPIO_CLR0 = 1<<17; // Set GPIO pin LOW
  setup_pwm(17);

  printf("\n>>> "); fflush(stdout);
  // motor B input is still low, so motor gets power when pwm input A is high
  force_pwm0(0,PWM0_ENABLE);
  // start motor off slow (low most of the time) then ramp up speed
  // (increasing the high part of the pulses)
  for (s=0x100; s<=0x400; s+=0x10)
    { long_wait(6);
      set_pwm0(s);
      putchar('+'); fflush(stdout);
    }
  long_wait(10);
  // now slow the motor down by decreasing the time the pwm is high
  for (s=0x400; s>=0x100; s-=0x10)
    { long_wait(6);
      set_pwm0(s);
      putchar('-'); fflush(stdout);
    }
  // make sure motor is stopped
  pwm_off();
  // same in reverse direction
  // set motor B input to high, so motor gets power when pwm input A is low
  GPIO_SET0 = 1<<17;
  // when we enable pwm with reverse polarity, a pwm value near 0 means
  // that the LOW phase is only done for a short period amount of time
  // and a pwm value near 0x400 (the max we set in setup_pwm) means
  // that the LOW phase is done for a long period amount of time
  force_pwm0(0,PWM0_ENABLE|PWM0_REVPOLAR);
  printf("\n<<< ");
  // as before we ramp up speed of motor
  for (s=0x100; s<=0x400; s+=0x10)
    { long_wait(6);
      set_pwm0(s);
      putchar('+'); fflush(stdout);
    }
  long_wait(10);
  // then slow it down
  for (s=0x400; s>=0x100; s-=0x10)
    { long_wait(6);
      set_pwm0(s);
      putchar('-'); fflush(stdout);
    }
  GPIO_CLR0 = 1<<17;
  pwm_off();
  putchar('\n');

  restore_io();
}
