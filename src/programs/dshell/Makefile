CC=gcc

#CFLAGS=-Wall -nostdlib -lgcc -nostartfiles -ffreestanding -fpic
#LFLAGS=-shared

CFLAGS=-Wall

OBJS=dshell.o

EXE=dshell

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
