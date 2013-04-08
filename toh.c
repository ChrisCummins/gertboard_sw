/* -*- indent-tabs-mode: nil; eval: (c-set-style "gnu"); -*-
 *
 * Tower of Hanoi
 *
 * This file is part of the gertboard test suite
 *
 * Copyright (C) Chris Cummins 2013
 * No rights reserved
 * You may treat this program as if it was in the public domain
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * If we don't explicitly declare keyboard input, assume we are compiling with
 * the Raspberry Pi's GPIO as input device.
 */
#ifndef KEYBOARD_INPUT
# define GERTBOARD_INPUT
#endif

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

/*
 * Only include the gertboard header if necessary. This keeps the program
 * architecture-agnostic, allowing for builds on x86 and other systems.
 */
#ifdef GERTBOARD_INPUT
# include "gb_common.h"
#endif

/*
 * Utilize GCC's branch prediction hints when possible.
 */
#ifdef __GNUC__
# define likely(x)    __builtin_expect((x),1)
# define unlikely(x)  __builtin_expect((x),0)
#else
# define likely(x)    (x)
# define unlikely(x)  (x)
#endif

#define MAX_DISKS (USHRT_MAX - 2)
typedef unsigned short disk_t;

enum rod_e {
  ROD_A,
  ROD_B,
  ROD_C,
  ROD_MAX
};

static disk_t         rods[ROD_MAX][MAX_DISKS];
static disk_t         disk_in_hand = 0;
static unsigned int   disk_count = 3;
static unsigned long  move_counter = 0;
static unsigned long  optimal;

/*
 * Backend dependent functions. We declare them here to enforce the
 * prototype, leaving the actual implementation for the individual
 * backends.
 */
static void           init_input_backend ();
static enum rod_e     get_next_action ();

#ifdef GERTBOARD_INPUT

/*
 * Gertboard Input Backend.
 *
 * This code uses switches S1, S2 and S3 on the gertboard as the input for the
 * game, with a simple backend driver which polls the GPIO_IN0 port and
 * determines new button press actions based its value.
 */

#define GPIO_PULL_UP  2

#define SET_GPIO(gpio, val)                     \
  (gpio) = (val);                               \
  short_wait ()

#define IS_BUTTON(x) (x == 4 || x == 2 || x == 1)

static unsigned int b, lb = 0, pb = 0x10;

static inline void
gpio_set_pull (unsigned pull)
{
  SET_GPIO (GPIO_PULL, GPIO_PULL_UP);
  SET_GPIO (GPIO_PULLCLK0, 0x03800000);
  GPIO_PULL = 0;
  GPIO_PULLCLK0 = 0;
}

static void
sig_handler (int sig)
{
  if (sig == SIGINT)
    {
      SET_GPIO (GPIO_PULL, 0);
      gpio_set_pull (0);
      exit (0);
    }
}

static inline void
init_input_backend ()
{
  printf ("These are the connections for the Tower of Hanoi game:\n");
  printf ("GP25 in J2 --- B1 in J3\n");
  printf ("GP24 in J2 --- B2 in J3\n");
  printf ("GP23 in J2 --- B3 in J3\n");
  printf ("When ready hit enter.\n");
  getchar ();

  setup_io ();

  INP_GPIO (23);
  INP_GPIO (24);
  INP_GPIO (25);

  gpio_set_pull (GPIO_PULL_UP);

  if (unlikely (signal(SIGINT, sig_handler) == SIG_ERR))
    {
      fprintf (stderr, "Failed to attach signal handler to SIGINT.\n");
      abort ();
    }
}

static inline enum rod_e
translate_button (unsigned int b)
{
  assert (IS_BUTTON (b));

  switch (b)
    {
    case 4:
      return ROD_A;
      break;
    case 2:
      return ROD_B;
      break;
    case 1:
      return ROD_C;
      break;
    default:
      fprintf (stderr, "Invalid button: [%d]!\n", b);
      abort ();
      break;
    }
}

static void
pause_thread (unsigned int ticks)
{
  unsigned int t;

  for (t = 0; t <= ticks; t++)
    short_wait ();
}

static enum rod_e
get_next_action ()
{
  while (1)
    {
      b = (~GPIO_IN0 >> 23) & 0x07;
      if ((IS_BUTTON (b) || b == 0) && b ^ pb)
        {
          pb = b;

          if (b && b ^ lb)
            {
              lb = b;
              return translate_button (b);
            }
          else
            lb = b;

          pause_thread (100);
        }
      else
        pb = b;
    }
}

#elif KEYBOARD_INPUT

/*
 * Keyboard Input Backend.
 *
 * This backend collects input events from the stdin stream, so does not make
 * use of the Raspberry Pi's GPIO or the Gertboard. This backend can be used to
 * execute the toh program on alternative architectures, such as x86. To enable
 * it, the KEYBOARD_INPUT macro must explicity defined when compiling the object
 * file. The easiest way to to do this is by setting the TOH variable to define
 * this when invoking make:
 *
 *  $ make TOH=-DKEYBOARD_INPUT
 */

static struct termios ot, nt;

/* There is no setup required for this backend, as we using function calls
 * provided by the standard library. */
static inline void init_input_backend () {}

static char
getch()
{
  char c;

  tcgetattr (0, &ot);
  nt = ot;
  nt.c_lflag &= ~ICANON;
  nt.c_lflag &= ~ECHO;
  tcsetattr (0, TCSANOW, &nt);
  c = getchar ();
  tcsetattr (0, TCSANOW, &ot);

  return c;
}


static enum rod_e
get_next_action ()
{
  int b;

  while (1)
    {
      b = getch () - 0x30;

      if (b >= 1 && b <= ROD_MAX)
        return b - 1;
    }
}

#endif /* KEYBOARD_INPUT */

static void
setup_new_game ()
{
  disk_t i, j;

  /* Initialise rods. */
  for (j = 0; j < ROD_MAX; j++)
    {
      for (i = 1; i < MAX_DISKS + 1; i++)
        rods[j][i] = 0;
      rods[j][0] = 1;
    }

  /* Initialise disks. */
  for (i = 1; i <= disk_count; i++)
    rods[0][i] = disk_count + 1 - i;

  rods[0][0] = disk_count;

  /* Determine the minimum number of moves for the puzzle size. */
  optimal = ((unsigned long) powl (2, disk_count)) - 1;

  printf ("A %d disk puzzle, this can be solved in %lu moves.\n\n",
          disk_count, optimal);
}

static disk_t
peek_disk_index (enum rod_e rod)
{
  return rods[rod][0];
}

static disk_t
peek_disk (enum rod_e rod)
{
  return rods[rod][peek_disk_index (rod)];
}

static disk_t
pop_disk (enum rod_e rod)
{
  disk_t i, d;

  i = peek_disk_index (rod);
  d = rods[rod][i];
  rods[rod][i] = 0;

  if (i > 1)
    rods[rod][0]--;

  return d;
}

/* Push disk onto rod. If successful, returns the size of the disk. If the top
 * disk on the rod is smaller than the new one, the size of the smaller disk
 * will be returned. */
static disk_t
push_disk (enum rod_e rod, disk_t disk)
{
  disk_t i, d;

  i = peek_disk_index (rod);
  d = rods[rod][i];

  if (d > 1)
    {
      if (d > disk)
        {
          d = rods[rod][i + 1] = disk;
          rods[rod][0]++;
        }
    }
  else
    d = rods[rod][1] = disk;

  return d;
}

static inline int
is_endgame ()
{
  return (rods[ROD_MAX - 1][0] == disk_count) ? 1 : 0;
}

static inline void
clear_screen ()
{

  printf ("\e[1;1H\e[2J");
}

static void
print_game_status ()
{
  int i, j;

  for (j = 0; j < ROD_MAX; j++)
    printf ("   %c   ", j + 0x41);
  printf ("\n");

  for (i = disk_count + 1; i > 0; i--)
    {
      for (j = 0; j < ROD_MAX; j++)
        {
          if (rods[j][i])
            printf ("  [%d]  ", rods[j][i]);
          else
            printf ("   |   ");
        }
      printf ("\n");
    }

  for (j = 0; j < ROD_MAX; j++)
    printf ("  ---  ");
  printf ("\n");
}

static void
perform_action (enum rod_e rod)
{
  disk_t d, new_d;

  clear_screen ();

  if (disk_in_hand)
    {
      d = peek_disk (rod);
      new_d = push_disk (rod, disk_in_hand);

      if (d == 0 || new_d == disk_in_hand)
        {
          printf ("Placed disk %d on rod %c.\n", disk_in_hand, rod + 0x41);
          disk_in_hand = 0;
        }
      else
        printf ("Cannot place disk %d on top of disk %d!\n", disk_in_hand, peek_disk (rod));
    }
  else
    {
      move_counter++;

      if ((disk_in_hand = pop_disk (rod)))
        printf ("Picked up disk %d from rod %c.\n", disk_in_hand, rod + 0x41);
      else
        printf ("No disks on rod %c!\n", rod + 0x41);
    }

  printf ("\n");
  print_game_status ();

  if (is_endgame ())
    {
      printf ("\nCongratulations! ");
      printf ("You completed the puzzle in %lu moves (%.0f%%).\n",
              move_counter, ((double) optimal / (double) move_counter) * 100);
      exit (0);
    }
}

int
main (int argc, char **argv)
{
  enum rod_e r;

  init_input_backend ();

  clear_screen ();

  setup_new_game ();
  print_game_status ();
  printf ("\n");

  while (1)
    {
      r = get_next_action ();
      perform_action (r);
    }

  return 0;
}
