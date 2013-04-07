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
#include "gb_spi.h"
#include "gb_pwm.h"

// potentiometer - motor test GPIO mapping:
//         Function            Mode
// GPIO0=  unused
// GPIO1=  unused
// GPIO4=  unused
// GPIO7=  unused
// GPIO8=  unused
// GPIO8=  SPI chip select A   Alt. 0
// GPIO9=  SPI MISO            Alt. 0
// GPIO10= SPI MOSI            Alt. 0
// GPIO11= SPI CLK             Alt. 0
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
  // setup GPIO 8 to 11 for SPI bus use
  INP_GPIO(8);  SET_GPIO_ALT(8,0);
  INP_GPIO(9);  SET_GPIO_ALT(9,0);
  INP_GPIO(10); SET_GPIO_ALT(10,0);
  INP_GPIO(11); SET_GPIO_ALT(11,0);
  // GPIO 17 is used for the MOTB input and is just high or low 
  // (depending on potentiometer input)
  INP_GPIO(17);  OUT_GPIO(17);
  // GPIO 18 is set up for using the pulse width modulator
  INP_GPIO(18);  SET_GPIO_ALT(18, 5);
} // setup_gpio


void main()
{ int r, s, v, fwd;

  printf ("These are the connections for the potentiometer - motor test:\n");
  printf ("jumper connecting GP11 to SCLK\n");
  printf ("jumper connecting GP10 to MOSI\n");
  printf ("jumper connecting GP9 to MISO\n");
  printf ("jumper connecting GP8 to CSnA\n");
  printf ("Potentiometer connections:\n");
  printf ("  (call 1 and 3 the ends of the resistor and 2 the wiper)\n");
  printf ("  connect 3 to 3V3\n");
  printf ("  connect 2 to AD0\n");
  printf ("  connect 1 to GND\n");
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

  // Set up GPIO pins for both A/D and motor
  setup_gpio();

  // Setup SPI bus
  setup_spi();

  // set pin controlling the non-PWM driver to low and get PWM ready
  GPIO_CLR0 = 1<<17; // Set GPIO pin LOW
  setup_pwm(17);

  // motor B input is still low, so motor gets power when pwm input A is high
  force_pwm0(0,PWM0_ENABLE);
  // we call "forward" the direction we get when the B input is high
  // we start out set up for going "backwards"
  fwd = 0;

  for (r=0; r<1200000; r++)
  {
    v= read_adc(0);
    if (v <= 511)
      { 
      // map 0 to 511 to going "backwards" -- 0 (one end of your pot) means
      // go backwards fast (v sent to PWM is near 1023), as we increase 
      // towards 510, motor speed slows, and at 511 (middle) motor is stopped
      // (v sent to PWM is near 0)
      v = 1023-(v * 2);
      // we want v near 0 to mean motor slow/stopped and v near 1023 to
      // mean motor going "backwards" fast
      if (fwd)
      { // going in the wrong direction
        // reverse polarity
        GPIO_SET0 = 1<<17;
	// We set PWM0_REVPOLAR flag below because normally a high value for
	// v means high cycle which means signal high most of the time.
	// But with motor input B high, this would mean that motor is slow,
        // which is not what we want. Setting PWM0_REVPOLAR flips the 
	// polarity so that a high v means that the signal is low most
	// of the time, which gives us a high speed.
        force_pwm0(v,PWM0_ENABLE|PWM0_REVPOLAR);
        fwd = 0;
      }
      else
        set_pwm0(v);
    }
    else
    {   
      // map A/D value of 512 to 1023 to going "forwards" -- at 512 (middle)
      // motor is stopped (v sent to PWM is near 0), as we increase A/D value
      // motor speed increases (in the "forwards" direction), and when A/D
      // value is at 1023 (at the "other" end of your pot), we send PWM a
      // value near 1023 so it goes very fast "forwards".
      v = (v-512)*2;
      if (!fwd)
      { // going in the wrong direction
        // reverse polarity
        GPIO_CLR0 = 1<<17;
	// Now normal polarity works for us: 
	// With a low v sent to PWM we get a low duty cycle, power
        // is off most of the time, and since motor b input is low this 
	// means a slow motor; when v goes to near 1023 we get a high duty
	// cycle which means power on most of the time which results in 
	// motor going quickly
        force_pwm0(v,PWM0_ENABLE);
        fwd = 1;
      }
      else
        set_pwm0(v);
    }
    short_wait();
  } // repeated read

  // set motor A and B inputs to 0 so motor stops
  GPIO_CLR0 = 1<<17;
  force_pwm0(0,PWM0_ENABLE);


  restore_io();
}
