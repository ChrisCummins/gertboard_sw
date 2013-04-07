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

all : buttons butled leds ocol atod dtoa dad motor potmot decoder

clean :
	rm *.o buttons butled leds ocol atod dtoa dad motor potmot decoder

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

# The next lines generate the various object files

gb_common.o : gb_common.c gb_common.h
	gcc -c gb_common.c

buttons.o : buttons.c gb_common.h
	gcc -c buttons.c

butled.o : butled.c gb_common.h
	gcc -c butled.c

leds.o : leds.c gb_common.h
	gcc -c leds.c

gb_spi.o : gb_spi.c gb_common.h gb_spi.h
	gcc -c gb_spi.c

gb_pwm.o : gb_pwm.c gb_common.h gb_pwm.h
	gcc -c gb_pwm.c

atod.o : atod.c gb_common.h gb_spi.h
	gcc -c atod.c

dtoa.o : dtoa.c gb_common.h gb_spi.h
	gcc -c dtoa.c

dad.o : dad.c gb_common.h gb_spi.h
	gcc -c dad.c

motor.o : motor.c gb_common.h gb_pwm.h
	gcc -c motor.c

potmot.o : potmot.c gb_common.h gb_spi.h gb_pwm.h
	gcc -c potmot.c

ocol.o : ocol.c gb_common.h gb_spi.h
	gcc -c ocol.c

decoder.o : decoder.c gb_common.h
	gcc -c decoder.c

