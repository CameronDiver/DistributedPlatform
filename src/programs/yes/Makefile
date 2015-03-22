CC=../../dcc

CFLAGS=-Wall

OBJS=yes.o ../../stdlib/syscall.o ../../stdlib/stdio.o ../../stdlib/stdlib.o

EXE=yes.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
