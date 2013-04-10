/* -*- indent-tabs-mode: t; eval: (c-set-style "linux"); -*-
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
 * Only include the gertboard header if necessary. This keeps the program
 * architecture-agnostic, allowing for builds on x86 and other systems.
 */
#ifdef gertboard_BACKEND
#include "gb_common.h"
#endif

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*
 * Utilize GCC's branch prediction hints when possible.
 */
#ifdef __GNUC__
#define likely(x)    __builtin_expect((x),1)
#define unlikely(x)  __builtin_expect((x),0)
#else
#define likely(x)    (x)
#define unlikely(x)  (x)
#endif

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define mprintf(...) if (!(flags & FLAGS_QUIET)) printf(__VA_ARGS__)

#define is_even(x)            ((x) % 2)
#define is_legal_move(r1, r2) (peek_disk((r2)) > peek_disk((r1))        \
                               || peek_disk((r2)) == 0)
#define is_endgame()          (rods[ROD_MAX - 1][0] == disk_count)
#define clear_screen()        mprintf("\e[1;1H\e[2J")

#define MAX_DISKS (USHRT_MAX - 2)
typedef unsigned short disk_t;

enum rod_e {
	ROD_A,
	ROD_B,
	ROD_C,
	ROD_MAX
};

/*
 * Room is left here for future use when
 * additional arguments are implemented.
 */
enum flags_e {
	FLAGS_QUIET = 1 << 0,
} flags = 0;

static disk_t         rods[ROD_MAX][MAX_DISKS];
static disk_t         disk_in_hand = 0;
static unsigned int   disk_count = 3;
static unsigned long  move_counter = 0;
static unsigned long  optimal;

/* Game functions. */
#define       peek_disk_index(r) rods[(r)][0]
#define       peek_disk(r)       rods[(r)][peek_disk_index(r)]

static inline disk_t pop_disk(enum rod_e rod)
{
	disk_t i, d;

	i = peek_disk_index(rod);
	d = rods[rod][i];
	rods[rod][i] = 0;

	if (i > 1)
		rods[rod][0]--;

	return d;
}

/* Push disk onto rod. If successful, returns the size of the disk. If the top
 * disk on the rod is smaller than the new one, the size of the smaller disk
 * will be returned. */
static inline disk_t push_disk(enum rod_e rod, disk_t disk)
{
	disk_t i, d;

	i = peek_disk_index(rod);
	d = rods[rod][i];

	if (d > 1) {
		if (d > disk) {
			d = rods[rod][i + 1] = disk;
			rods[rod][0]++;
		}
	}
	else
		d = rods[rod][1] = disk;

	return d;
}

/*
 * Backend dependent functions. We declare them here to enforce the
 * prototype, leaving the actual implementation for the individual
 * backends.
 */
static void        init_input_backend ();
static enum rod_e  get_next_action ();

#ifdef gertboard_BACKEND

/*
 * Gertboard Input Backend.
 *
 * This code uses switches S1, S2 and S3 on the gertboard as the input for the
 * game, with a simple backend driver which polls the GPIO_IN0 port and
 * determines new button press actions based its value.
 */

#define GPIO_PULL_UP  2

#define set_gpio(gpio, val)			\
	(gpio) = (val);				\
	short_wait()

#define is_button(x) (x == 4 || x == 2 || x == 1)

#define gpio_set_pull(p)			\
	set_gpio(GPIO_PULL, (p));		\
	set_gpio(GPIO_PULLCLK0, 0x03800000);	\
	GPIO_PULL = 0;				\
	GPIO_PULLCLK0 = 0

static unsigned int b, lb = 0, pb = 0x10;

static void sig_handler(int sig)
{
	if (sig == SIGINT) {
		set_gpio(GPIO_PULL, 0);
		gpio_set_pull(0);
		exit(EXIT_SUCCESS);
	}
}

static inline void init_input_backend()
{
	printf("These are the connections for the Tower of Hanoi game:\n");
	printf("GP25 in J2 --- B1 in J3\n");
	printf("GP24 in J2 --- B2 in J3\n");
	printf("GP23 in J2 --- B3 in J3\n");
	printf("When ready hit enter.\n");
	getchar();

	setup_io();

	INP_GPIO(23);
	INP_GPIO(24);
	INP_GPIO(25);

	gpio_set_pull(GPIO_PULL_UP);

	if (unlikely(signal(SIGINT, sig_handler) == SIG_ERR)) {
		eprintf("Failed to attach signal handler to SIGINT.\n");
		abort();
	}
}

static inline enum rod_e translate_button(unsigned int b)
{
	assert(is_button(b));

	switch (b) {
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
		eprintf("Invalid button: [%d]!\n", b);
		abort();
		break;
	}
}

static void pause_thread(unsigned int ticks)
{
	unsigned int t;

	for (t = 0; t <= ticks; t++)
		short_wait();
}

static enum rod_e get_next_action ()
{
	while (1) {
		b = (~GPIO_IN0 >> 23) & 0x07;
		if ((is_button(b) || b == 0) && b ^ pb) {
			pb = b;

			if (b && b ^ lb) {
				lb = b;
				return translate_button(b);
			} else
				lb = b;

			pause_thread(100);
		} else
			pb = b;
	}
}

#elif keyboard_BACKEND

/*
 * Keyboard Input Backend.
 *
 * This backend collects input events from the stdin stream, so does not make
 * use of the Raspberry Pi's GPIO or the Gertboard. This backend can be used to
 * execute the toh program on alternative architectures, such as x86. To enable
 * it, the keyboard_BACKEND macro must explicity defined when compiling the
 * object file. The easiest way to to do this is by setting the BACKEND variable
 * when invoking make:
 *
 *  $ make BACKEND=keyboard
 */

static struct termios ot, nt;

/* There is no setup required for this backend, as we using function calls
 * provided by the standard library. */
static inline void init_input_backend()
{
}

static char getch()
{
	char c;

	tcgetattr(0, &ot);
	nt = ot;
	nt.c_lflag &= ~ICANON;
	nt.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &nt);
	c = getchar();
	tcsetattr(0, TCSANOW, &ot);

	return c;
}


static enum rod_e get_next_action()
{
	int b;

	while (1) {
		b = getch() - 0x30;

		if (b >= 1 && b <= ROD_MAX)
			return b - 1;
	}
}

#elif autonomous_BACKEND

/*
 * Autonomous Backend.
 *
 * This autonomous backend includes an algorithm for determining the option next
 * action and will solve any game in the minimum possible number of moves. This
 * backend in architecture agnostic, so can be compiled on x86. To enable it,
 * the autonomous_BACKEND macro must explicity defined when compiling the object
 * file. The easiest way to to do this is by setting the BACKEND variable when
 * invoking make:
 *
 *  $ make BACKEND=autonomous
 */

static enum direction_e {
	MOVE_LEFT  = 2,
	MOVE_RIGHT = 1
} direction;

static enum rod_e lr = 0, next_rod = 0;

/* There is no setup required for this backend. */
static inline void init_input_backend()
{
}

static void stutter()
{
	int i, j = 0, k = 0;

	for (i = 0; i < 9000000; i++) {
		j++;
		k--;
	}
}

static inline enum rod_e determine_next_rod()
{
	enum rod_e base, probe;

	for (base = 0; base < ROD_MAX; base++)
		if (base ^ lr && peek_disk(base))
			for (probe = 0; probe < ROD_MAX; probe++)
				if (probe ^ base && probe ^ lr)
					if (is_legal_move(base, probe)) {
						next_rod = probe;
						goto found_solution;
					}
found_solution:
	return base;
}

static enum rod_e get_next_action()
{
	if (!(flags & FLAGS_QUIET)) {
		int i;

		for (i = 0; i < 20; i++)
			stutter();
	}

	/* Determine the direction to move the smallest disk. */
	if (!direction)
		direction = (is_even(disk_count)) ? MOVE_LEFT : MOVE_RIGHT;

	if (disk_in_hand) {
		return (is_even(move_counter))
			? next_rod
			: (lr = ((lr + direction) % ROD_MAX));
	} else {
		return (is_even(move_counter))
			? determine_next_rod()
			: lr;
	}
}

#else
#error "Incorrect or missing backend!"
#endif /* End of backend-conditional code. */

static void setup_new_game()
{
	disk_t i, j;

	/* Initialise rods. */
	for (j = 0; j < ROD_MAX; j++) {
		for (i = 1; i < MAX_DISKS + 1; i++)
			rods[j][i] = 0;
		rods[j][0] = 1;
	}

	/* Initialise disks. */
	for (i = 1; i <= disk_count; i++)
		rods[0][i] = disk_count + 1 - i;

	rods[0][0] = disk_count;

	/* Determine the minimum number of moves for the puzzle size. */
	optimal = ((unsigned long)powl(2, disk_count)) - 1;

        printf("A %d disk puzzle, this can be solved in %lu moves.",
               disk_count, optimal);
        mprintf("\n\n");
}

static void print_game_status()
{
	int i, j;

	for (j = 0; j < ROD_MAX; j++)
		mprintf("   %c	 ", j + 0x41);
	mprintf("\n");

	for (i = disk_count + 1; i > 0; i--) {
		for (j = 0; j < ROD_MAX; j++) {
			if (rods[j][i]) {
				mprintf("  [%d]	 ", rods[j][i]);
			} else {
				mprintf("   |	");
			}
		}
		mprintf("\n");
	}

	for (j = 0; j < ROD_MAX; j++)
		mprintf("  ---	");

	mprintf("\n\n Moves taken: %3lu / %lu\n", move_counter, optimal);
}

static void perform_action(enum rod_e rod)
{
	disk_t d, new_d;

	clear_screen();

	if (disk_in_hand) {
		d = peek_disk(rod);
		new_d = push_disk(rod, disk_in_hand);

		if (d == 0 || new_d == disk_in_hand) {
			mprintf("Placed disk %d on rod %c.\n",
				disk_in_hand, rod + 0x41);
			disk_in_hand = 0;
			move_counter++;
		} else
			mprintf("Cannot place disk %d on top of disk %d!\n",
				disk_in_hand, peek_disk(rod));
	} else {
		if ((disk_in_hand = pop_disk(rod))) {
			mprintf("Picked up disk %d from rod %c.\n",
				disk_in_hand, rod + 0x41);
		} else {
			mprintf("No disks on rod %c!\n", rod + 0x41);
		}
	}
	mprintf("\n");
	print_game_status();
}

static inline void run_game()
{
	enum rod_e r;
	time_t start, stop;
	clock_t clockbegin, clockend;
	double systemtime, totaltime;

	/* Don't start the clock until we've made the first move. */
	r = get_next_action();
	clockbegin = clock();
	time(&start);

	while (1) {
		perform_action(r);

		if (unlikely(is_endgame()))
			break;

		r = get_next_action();
	}

	clockend = clock();
	time(&stop);

	systemtime = ((double) clockend - (double) clockbegin) / CLOCKS_PER_SEC;
	totaltime = difftime(stop, start);

	printf("\nCongratulations! Puzzle completed in %lu moves (%.0f%%).\n",
	       move_counter, ((double) optimal / (double) move_counter) * 100);
	printf("Time:  %.2fs system, %.0fs total.\n",
	       systemtime, totaltime);
	printf("Speed: %.2fmps system, %0.2fmps total.\n",
	       (double) move_counter / systemtime,
	       (double) move_counter / totaltime);
}

int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, ":qd:")) != -1) {
		switch (c) {
		case 'q':
			flags |= FLAGS_QUIET;
			break;
		case 'd':
			disk_count = atoi(optarg);
			if (disk_count < 2 || disk_count > MAX_DISKS) {
				eprintf("The number of disks must be in the "
					"range [2, %d].\n", MAX_DISKS);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			eprintf("Usage: %s [-q] [-d <ndisks>]\n", argv[0]);
			exit (EXIT_FAILURE);
		}
	}

	init_input_backend();

	clear_screen();
	setup_new_game();
	print_game_status();
	mprintf("\n");

	run_game();

	return 0;
}
