//
// Gertboard test suite
//
// pwm header file
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
// Try to strike a balance between keep code simple for
// novice programmers but still have reasonable quality code
// (e.g. do not use #ifndef big_demo_h....)
//


#define PWMCLK_CNTL  *(clk+40)
#define PWMCLK_DIV   *(clk+41)

#define PWM_CONTROL *pwm
#define PWM_STATUS  *(pwm+1)
#define PWM0_RANGE  *(pwm+4)
#define PWM1_RANGE  *(pwm+8)
#define PWM0_DATA   *(pwm+5)
#define PWM1_DATA   *(pwm+9)

// PWM Control register bits
#define PWM1_MS_MODE    0x8000  // Run in MS mode
#define PWM1_USEFIFO    0x2000  // Data from FIFO
#define PWM1_REVPOLAR   0x1000  // Reverse polarity
#define PWM1_OFFSTATE   0x0800  // Ouput Off state
#define PWM1_REPEATFF   0x0400  // Repeat last value if FIFO empty
#define PWM1_SERIAL     0x0200  // Run in serial mode
#define PWM1_ENABLE     0x0100  // Channel Enable

#define PWM0_MS_MODE    0x0080  // Run in MS mode
#define PWM0_USEFIFO    0x0020  // Data from FIFO
#define PWM0_REVPOLAR   0x0010  // Reverse polarity
#define PWM0_OFFSTATE   0x0008  // Ouput Off state
#define PWM0_REPEATFF   0x0004  // Repeat last value if FIFO empty
#define PWM0_SERIAL     0x0002  // Run in serial mode
#define PWM0_ENABLE     0x0001  // Channel Enable

#define PWM_CLRFIFO     0x0040  // Clear FIFO (Self clearing bit)

// PWM status bits I need
#define PWMS_BUSERR     0x0100  // Register access was too fast
// (Write to clear it)

// declarations for routines
void setup_pwm();
void force_pwm0(int, int);
void pwm_off();
