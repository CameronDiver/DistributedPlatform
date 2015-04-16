CC=../../../dcc

CFLAGS=-Wall

OBJS=init.o ../../../stdlib/syscall.o ../../../stdlib/stdio.o ../../../stdlib/stdlib.o ../../../stdlib/sys.o

EXE=../bin/init

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
