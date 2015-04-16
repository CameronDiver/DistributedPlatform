CC=../../../dcc

CFLAGS=-Wall

OBJS=factor.o ../../../stdlib/syscall.o ../../../stdlib/stdio.o ../../../stdlib/stdlib.o

EXE=../bin/factor

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
