CC = gcc
RM = rm -f
LIBS = -lm -lIL -lILU `pkg-config --libs gtk+-2.0` `pkg-config --libs libglade-2.0` `pkg-config --libs gthread-2.0`

# Tunables: NO_ASM, SKIP_GTK
DEFS = -D NO_ASM

# Common gcc flags
COMMON_CFLAGS = -Wall -std=c99 `pkg-config --cflags gtk+-2.0` `pkg-config --cflags libglade-2.0`

# Non-debugging / non-optimised
#CFLAGS = ${COMMON_CFLAGS}

# Debugging
#CFLAGS = ${COMMON_CFLAGS} -DDEBUG -g
#CFLAGS = ${COMMON_CFLAGS} -DDEBUG -g -pg

# Optimized

# Impact of various GCC optimisation flags ont FPS with bilinear filtering enabled: 
#
# Without -fomit-frame-pointer 	slightly slower.
# Without -ffast-math      		about 35-40% slower.
# Without -funroll-loops   		slightly *faster*.
# Without -march=prescott  		75% slower.
# Without -O2			  		60% slower.

CFLAGS = ${COMMON_CFLAGS} ${DEFS} -march=nocona -O2 -fomit-frame-pointer -ffast-math
#CFLAGS = ${COMMON_CFLAGS} ${DEFS} -march=prescott -O2 -ffast-math -fno-inline -pg

#OBJECTS = main.o texture.o math.o context.o draw.o matrix.o draw_asm.o camera.o
OBJECTS = main.o texture.o math.o context.o draw.o matrix.o camera.o
LISTINGS = draw.s

all: glum

glum: ${OBJECTS}
	${CC} ${CFLAGS} -rdynamic ${LIBS} ${OBJECTS} -o glum

draw_asm.o: draw_asm.asm
	nasm -g -f elf draw_asm.asm

main.o: main.c glum.h texture.h test.tris test2.tris test3.tris
	${CC} ${CFLAGS} -c main.c

texture.o: texture.c texture.h
	${CC} ${CFLAGS} -c texture.c

math.o: math.c math.h
	${CC} ${CFLAGS} -c math.c

context.o: context.h context.c
	${CC} ${CFLAGS} -c context.c

draw.o: draw.h draw.c
	${CC} ${CFLAGS} -c draw.c
#	${CC} ${CFLAGS} -S draw.c

matrix.o: matrix.h matrix.c
	${CC} ${CFLAGS} -c matrix.c

camera.o: camera.h camera.c
	${CC} ${CFLAGS} -c camera.c

clean:
	${RM} glum gmon.out ${OBJECTS} ${LISTINGS} *~

test:
	@-./glum

.PHONY: test


