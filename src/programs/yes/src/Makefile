CC=../../../dcc

CFLAGS=-Wall

OBJS=yes.o ../../../stdlib/syscall.o ../../../stdlib/stdio.o ../../../stdlib/stdlib.o

EXE=../bin/yes

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
