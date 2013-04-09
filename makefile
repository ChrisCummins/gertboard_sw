#
# Makefile for gertboard test programs
#
#
# Copyright (C) Gert Jan van Loo & Myra VanInwegen 2012
# No rights reserved
# You may treat this program as if it was in the public domain
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Keep things simple for novice users
# so do not use any implicit rules!
#

CFLAGS=-Wall -W -Wuninitialized -Wextra -Wno-unused-parameter -g -O0

all : buttons butled leds ocol atod dtoa dad motor potmot decoder toh

clean :
	rm -f *.o buttons butled leds ocol atod dtoa dad motor potmot decoder toh

buttons : gb_common.o buttons.o
	gcc -o buttons gb_common.o buttons.o

butled : gb_common.o butled.o
	gcc -o butled gb_common.o butled.o

leds : gb_common.o leds.o
	gcc -o leds gb_common.o leds.o

ocol : gb_common.o ocol.o
	gcc -o ocol gb_common.o ocol.o

atod : gb_common.o gb_spi.o atod.o
	gcc -o atod gb_common.o gb_spi.o atod.o

dtoa : gb_common.o gb_spi.o dtoa.o
	gcc -o dtoa gb_common.o gb_spi.o dtoa.o

dad : gb_common.o gb_spi.o dad.o
	gcc -o dad gb_common.o gb_spi.o dad.o

motor : gb_common.o gb_pwm.o motor.o
	gcc -o motor gb_common.o gb_pwm.o motor.o

potmot : gb_common.o gb_pwm.o gb_spi.o potmot.o
	gcc -o potmot gb_common.o gb_pwm.o gb_spi.o potmot.o

decoder : gb_common.o decoder.o
	gcc -o decoder gb_common.o decoder.o

toh : gb_common.o toh.o
	gcc $(CFLAGS) -o toh gb_common.o toh.o -lm

# The next lines generate the various object files

gb_common.o : gb_common.c gb_common.h
	gcc $(CFLAGS) -c gb_common.c

buttons.o : buttons.c gb_common.h
	gcc $(CFLAGS) -c buttons.c

butled.o : butled.c gb_common.h
	gcc $(CFLAGS) -c butled.c

leds.o : leds.c gb_common.h
	gcc $(CFLAGS) -c leds.c

gb_spi.o : gb_spi.c gb_common.h gb_spi.h
	gcc $(CFLAGS) -c gb_spi.c

gb_pwm.o : gb_pwm.c gb_common.h gb_pwm.h
	gcc $(CFLAGS) -c gb_pwm.c

atod.o : atod.c gb_common.h gb_spi.h
	gcc $(CFLAGS) -c atod.c

dtoa.o : dtoa.c gb_common.h gb_spi.h
	gcc $(CFLAGS) -c dtoa.c

dad.o : dad.c gb_common.h gb_spi.h
	gcc $(CFLAGS) -c dad.c

motor.o : motor.c gb_common.h gb_pwm.h
	gcc $(CFLAGS) -c motor.c

potmot.o : potmot.c gb_common.h gb_spi.h gb_pwm.h
	gcc $(CFLAGS) -c potmot.c

ocol.o : ocol.c gb_common.h gb_spi.h
	gcc $(CFLAGS) -c ocol.c

decoder.o : decoder.c gb_common.h
	gcc $(CFLAGS) -c decoder.c

ifneq ($(BACKEND),)
backend_flags=-D$(BACKEND)_BACKEND
else
backend_flags=-Dgertboard_BACKEND
endif

toh.o : toh.c gb_common.h
	gcc $(CFLAGS) $(backend_flags) -c toh.c

# Tags rules

all_sources = $(wildcard *.c) $(wildcard *.h)

TAGS : $(all_sources)
	etags $^

tags : $(all_sources)
	ctags $^
