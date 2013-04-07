//=============================================================================
//
//
// Gertboard Demo
//
// main file
//
// This file is part of gertboard_demo.
//
//
// Copyright (C) Gert Jan van Loo 2012
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
//
// Try to strike a balance between keep code simple for
// novice programmers but still have reasonable quality code
//
// DEMO GPIO mapping:
//         Function            Mode
// GPIO0=  LED                 Output
// GPIO1=  LED                 Output
// GPIO4=  PWM channel-B       Output
// GPIO7=  SPI chip select B   Funct. 0
// GPIO8=  SPI chip select A   Funct. 0
// GPIO9=  SPI MISO            Funct. 0
// GPIO10= SPI MOSI            Funct. 0
// GPIO11= SPI CLK             Funct. 0
// GPIO14= UART TXD           (Funct. 0)
// GPIO15= UART RXD           (Funct. 0)
// GPIO17= LED                 Output
// GPIO18= PWM channel-A       Funct. 5
// GPIO21= LED                 Output
// GPIO22= LED                 Output
// GPIO23= LED                 Output
// GPIO24= Pushbutton          Input
// GPIO25= Pushbutton          Input
//
//
// Notes:
// 1/ In some Linux systems (e.g. Debian) the UART is used by Linux.
//    So for now do not demo the UART.
// 2/ At the moment (16-March-2012) there is no Linux driver for
//    the audio yet so the PWM is free.
//    This is likely to change and in that case the Linux
//    audio/PWM driver must be disabled.
//
//

#include "gertboard_demo.h"

#define BCM2708_PERI_BASE        0x20000000
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* Clocks */
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO   */
#define PWM_BASE                 (BCM2708_PERI_BASE + 0x20C000) /* PWM    */
#define SPI0_BASE                (BCM2708_PERI_BASE + 0x204000) /* SPI0 controller */
#define UART0_BASE               (BCM2708_PERI_BASE + 0x201000) /* Uart 0 */
#define UART1_BASE               (BCM2708_PERI_BASE + 0x215000) /* Uart 1 (not used) */

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

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
char *clk_mem,  *clk_map;
char *gpio_mem, *gpio_map;
char *pwm_mem,  *pwm_map;
char *spi0_mem, *spi0_map;
char *uart_mem, *uart_map;

// I/O access
volatile unsigned *gpio;
volatile unsigned *pwm;
volatile unsigned *clk;
volatile unsigned *spi0;
volatile unsigned *uart;


//
//  GPIO
//

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET0   *(gpio+7)  // Set GPIO high bits 0-31
#define GPIO_SET1   *(gpio+8)  // Set GPIO high bits 32-53

#define GPIO_CLR0   *(gpio+10) // Set GPIO low bits 0-31
#define GPIO_CLR1   *(gpio+11) // Set GPIO low bits 32-53
#define GPIO_PULL   *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock


//
//  UART 0
//

#define UART0_BAUD_HI *(uart+9)
#define UART0_BAUD_LO *(uart+10)


void setup_io();
void restore_io();

//
// This is a software loop to wait
// a short while.
//
void short_wait()
{ int w;
  for (w=0; w<100; w++)
  { w++;
    w--;
  }
} // short_wait


//
// Simple SW wait loop
//
void long_wait(int v)
{ int w;
  while (v--)
    for (w=-800000; w<800000; w++)
    { w++;
      w--;
    }
} // long_wait


//
// Set GPIO pins to the right mode
// DEMO GPIO mapping:
//         Function            Mode
// GPIO0=  LED                 Output
// GPIO1=  LED                 Output
// GPIO4=  PWM channel-B       Output
// GPIO7=  SPI chip select B   Funct. 0
// GPIO8=  SPI chip select A   Funct. 0
// GPIO9=  SPI MISO            Funct. 0
// GPIO10= SPI MOSI            Funct. 0
// GPIO11= SPI CLK             Funct. 0
// GPIO14= UART TXD           (Funct. 0)
// GPIO15= UART RXD           (Funct. 0)
// GPIO17= LED                 Output
// GPIO18= PWM channel-A       Funct. 5
// GPIO21= LED                 Output
// GPIO22= LED                 Output
// GPIO23= LED                 Output
// GPIO24= Pushbutton          Input
// GPIO25= Pushbutton          Input
//
// Always call INP_GPIO(x) first
// as that is how the macros work
void setup_gpio()
{
   INP_GPIO(0);  OUT_GPIO(0);
   INP_GPIO(1);  OUT_GPIO(1);
   INP_GPIO(4);  OUT_GPIO(4);
   INP_GPIO(7);  SET_GPIO_ALT(7,0);
   INP_GPIO(8);  SET_GPIO_ALT(8,0);
   INP_GPIO(9);  SET_GPIO_ALT(9,0);
   INP_GPIO(10); SET_GPIO_ALT(10,0);
   INP_GPIO(11); SET_GPIO_ALT(11,0);
   // 14 and 15 are already set to UART mode
   // by Linux. Best if we don't touch them
   // INP_GPIO(14); SET_GPIO_ALT(14,0);
   // INP_GPIO(54); SET_GPIO_ALT(15,0);
   INP_GPIO(17);  OUT_GPIO(17);
   INP_GPIO(18);  SET_GPIO_ALT(18,5);
   INP_GPIO(21);  OUT_GPIO(21);
   INP_GPIO(22);  OUT_GPIO(22);
   INP_GPIO(23);  OUT_GPIO(23);
   INP_GPIO(24);
   INP_GPIO(25);

   // enable pull-up on GPIO24&25
   GPIO_PULL = 2;
   short_wait();
   // clock on GPIO 24 & 25 (bit 24 & 25 set)
   GPIO_PULLCLK0 = 0x03000000;
   short_wait();
   GPIO_PULL = 0;
   GPIO_PULLCLK0 = 0;
} // setup_gpio




int main(void)
{ char key;

   // Map the I/O sections
   setup_io();

   // Set ALL GPIO pins to the required mode
   setup_gpio();

   // Set up PWM module
   setup_pwm();

   // Setup the SPI
   setup_spi();

   // We don't touch the UART for now

   //
   // Here your main program can start
   //
   do {
     printf(" l/L : Walk the LEDS\n");
     printf(" b/B : Show buttons\n");
     printf(" m/M : Control the motor\n");
     printf(" a/A : Read the ADC values\n");
     printf(" c/C : ADC => Motor\n");
     printf("( D : Set  the DAC values\n");
     printf(" q/Q : Quit program\n");
     key = getchar();
     switch (key)
     {
     case 'l':
     case 'L':
         quick_led_demo();
         break;

     case 'b':
     case 'B':
         quick_buttons_demo();
         break;

     case 'm':
     case 'M':
         quick_pwm_demo();
         break;

     case 'a':
     case 'A':
         quick_adc_demo();
         break;

     case 'c':
     case 'C':
         adc_pwm_demo();
         break;

     case 0x0A:
     case 0x0D:
         // ignore CR/LF
         break;

     default:
       printf("???\n");
     }

   } while (key!='q' && key!='Q');

   // make sure everything is off!
   leds_off();
   pwm_off();

   restore_io();

   return 0;
} // main


//
// Set up memory regions to access the peripherals.
// This is a bit of 'magic' which you should not touch.
// It it also the part of the code which makes that
// you have to use 'sudo' to run this program.
//
void setup_io()
{

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("Can't open /dev/mem\n");
      printf("Did you forgot to use 'sudo .. ?'\n");
      exit (-1);
   }

   /*
    * mmap clock
    */
   if ((clk_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }
   if ((unsigned long)clk_mem % PAGE_SIZE)
     clk_mem += PAGE_SIZE - ((unsigned long)clk_mem % PAGE_SIZE);

   clk_map = (unsigned char *)mmap(
      (caddr_t)clk_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      CLOCK_BASE
   );

   if ((long)clk_map < 0) {
      printf("clk mmap error %d\n", (int)clk_map);
      exit (-1);
   }
   clk = (volatile unsigned *)clk_map;


   /*
    * mmap GPIO
    */
   if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }
   if ((unsigned long)gpio_mem % PAGE_SIZE)
     gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);

   gpio_map = (unsigned char *)mmap(
      (caddr_t)gpio_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      GPIO_BASE
   );

   if ((long)gpio_map < 0) {
      printf("gpio mmap error %d\n", (int)gpio_map);
      exit (-1);
   }
   gpio = (volatile unsigned *)gpio_map;

   /*
    * mmap PWM
    */
   if ((pwm_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }
   if ((unsigned long)pwm_mem % PAGE_SIZE)
     pwm_mem += PAGE_SIZE - ((unsigned long)pwm_mem % PAGE_SIZE);

   pwm_map = (unsigned char *)mmap(
      (caddr_t)pwm_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      PWM_BASE
   );

   if ((long)pwm_map < 0) {
      printf("pwm mmap error %d\n", (int)pwm_map);
      exit (-1);
   }
   pwm = (volatile unsigned *)pwm_map;

   /*
    * mmap SPI0
    */
   if ((spi0_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }
   if ((unsigned long)spi0_mem % PAGE_SIZE)
     spi0_mem += PAGE_SIZE - ((unsigned long)spi0_mem % PAGE_SIZE);

   spi0_map = (unsigned char *)mmap(
      (caddr_t)spi0_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      SPI0_BASE
   );

   if ((long)spi0_map < 0) {
      printf("spi0 mmap error %d\n", (int)spi0_map);
      exit (-1);
   }
   spi0 = (volatile unsigned *)spi0_map;

   /*
    * mmap UART
    */
   if ((uart_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }
   if ((unsigned long)uart_mem % PAGE_SIZE)
     uart_mem += PAGE_SIZE - ((unsigned long)uart_mem % PAGE_SIZE);

   uart_map = (unsigned char *)mmap(
      (caddr_t)uart_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      UART0_BASE
   );

   if ((long)uart_map < 0) {
      printf("uart mmap error %d\n", (int)uart_map);
      exit (-1);
   }
   uart = (volatile unsigned *)uart_map;

} // setup_io

//
// Undo what we did above
//
void restore_io()
{
  munmap(uart_map,BLOCK_SIZE);
  munmap(spi0_map,BLOCK_SIZE);
  munmap(pwm_map,BLOCK_SIZE);
  munmap(gpio_map,BLOCK_SIZE);
  munmap(clk_map,BLOCK_SIZE);
  // can't reliable return data
  // as the pntr may have been moved to 4K boundary
  // so leave it to OS to return the memory
  // free(uart_mem);
  // free(spi0_mem);
  // free(pwm_mem);
  // free(gpio_mem);
  // free(clk_mem );
} // restore_io

