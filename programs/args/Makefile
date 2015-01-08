CC=gcc

CFLAGS=-Wall -nostdlib -lgcc -nostartfiles -ffreestanding -fpic
LFLAGS=-shared

OBJS=args.o ../../stdlib/stdio.o ../../stdlib/stdlib.o

EXE=args.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
		@rm -f $(OBJS)
