CC=gcc

CFLAGS=-Wall -nostdlib -lgcc -nostartfiles -ffreestanding -fpic
LFLAGS=-shared

OBJS=factor.o ../../stdlib/stdio.o ../../stdlib/stdlib.o ../../stdlib/unistd.o ../../stdlib/syscall.o ../../stdlib/string.o ../../stdlib/math.o

EXE=factor.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
