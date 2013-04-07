//=============================================================================
//
//
// Gertboard Demo
//
// This code is part of the Gertboard test suite
// Pulse-Width-Modulation part
//
// Copyright (C) Gert Jan van Loo & Myra VanIwengen2012
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
//  ______        ____  __
// |  _ \ \      / /  \/  |
// | |_) \ \ /\ / /| |\/| |
// |  __/ \ V  V / | |  | |
// |_|     \_/\_/  |_|  |_|
//

// Beware of the following:
// 1/ Every write to a PWM register needs to be passed to the PWM clock
//    This may take a while and takes longer if the PWM clock is slow.
// 2/ On top of that the PWM does NOT pick up any new values unless the counter
//    reaches its end. So to quickly pick up a new value you have to disable it
//    and then enable it again.
// 3/ The output polarity and reverse polarity bits are effective also if
//    the PWM is disabled but not of there is no clock

// This is how we control a motor with a single PWM channel:
//
//  |\                     /|
//  | \                   / |
//  |  \                 /  |
//  |   >--A-(Motor)-B--<   |
//  |  /                 \  |
//  | /                   \ |
//  |/                     \|
//
//
//  One direction:
//     +---+      +---+      +---+      +---+
// A   |   |      |   |      |   |      |   |
//   ==+   +======+   +======+   +======+   +=======
//
// B
//   =============================================== (B is always low)
// The motor is driven (gets power) when A is high, so a PWM signal with
// A high for most of the time will make the motor go fast, and a PWM
// signal with A low for most for the time will make the mnotor turn
// slowly.
//
//
//  Other direction:
//   --+   +------+   +------+   +------+   +------
// A   |   |      |   |      |   |      |   |
//     +===+      +===+      +===+      +===+
//
//   ---------------------------------------------- (B is always high)
// B
//
// Here the situation is reversed: the motor is driven (but in the 
// opposite direction) when A is low. So a PWM signal with
// A low for most of the time will make the motor go fast, and a PWM
// signal with A high for most for the time will make the mnotor turn
// slowly.
//
// Off is both A and B low (or high, but I use low)
//
//

#include "gb_common.h"
#include "gb_pwm.h"


//
// Setup the Pulse Width Modulator
// It needs a clock and needs to be off
//
void setup_pwm()
{
   // Derive PWM clock direct from X-tal
   // thus any system auto-slow-down-clock-to-save-power does not effect it
   // The values below depends on the X-tal frequency!
   PWMCLK_DIV  = 0x5A000000 | (32<<12); // set pwm div to 32 (19.2/3 = 600KHz)
   PWMCLK_CNTL = 0x5A000011; // Source=osc and enable

   // Make sure PWM is off 
   PWM_CONTROL = 0;  short_wait();

   // I use 1024 steps for the PWM
   // (Just a nice value which I happen to like)
   PWM0_RANGE = 0x400;  short_wait();

} // setup_pwm

//
// Set PWM value
// This routine does not wait for the value to arrive
// If a new value comes in before it is picked up by the chip
// it will definitely be too fast for the motor to respond to it
//
void set_pwm0(int v)
{ // make sure value is in safe range
  if (v<0) v=0;
  if (v>0x400) v=0x400;
  PWM0_DATA = v;
} // set_pwm0

//
// Force PWM value update
// This routine makes sure the new value goes in.
// This is done by dis-abling the PWM, write the value
// and enable it again. This routine is weak as it
// uses a delay which is tested (but not guaranteed)
// Controls channel 0 only.
//
void force_pwm0(int v,int mode)
{ int w;
  // disable
  PWM_CONTROL  = 0;
  // wait for this command to get to the PWM clock domain
  // that depends on PWN clock speed
  // unfortunately there is no way to know when this has happened :-(
  short_wait();
  // make sure value is in safe range
  if (v<0) v=0;
  if (v>0x400) v=0x400;
  PWM0_DATA = v;
  short_wait();

  PWM_CONTROL  = mode;
  short_wait();
} // force_pwm0

void pwm_off()
{
  force_pwm0(0,0);
}
