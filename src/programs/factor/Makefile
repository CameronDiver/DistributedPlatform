CC=../../dcc

CFLAGS=-Wall

OBJS=factor.o ../../stdlib/syscall.o ../../stdlib/stdio.o ../../stdlib/stdlib.o ../../stdlib/math.o

EXE=factor.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
