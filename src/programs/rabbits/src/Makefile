CC=../../../dcc

CFLAGS=-Wall

OBJS=rabbits.o ../../../stdlib/syscall.o ../../../stdlib/stdlib.o ../../../stdlib/sys.o

EXE=../bin/rabbits

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
