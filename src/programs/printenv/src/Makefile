CC=../../../dcc

CFLAGS=-Wall

OBJS=printenv.o ../../../stdlib/syscall.o ../../../stdlib/stdlib.o ../../../stdlib/stdio.o

EXE=../bin/printenv

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
