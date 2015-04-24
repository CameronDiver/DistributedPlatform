CC=../../../dcc

CFLAGS=-Wall

OBJS=shell.o ../../../stdlib/syscall.o ../../../stdlib/stdlib.o ../../../stdlib/sys.o

EXE=../bin/shell

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
