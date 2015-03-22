CC=../../dcc

CFLAGS=-Wall

OBJS=false.o ../../stdlib/syscall.o ../../stdlib/stdlib.o

EXE=false.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
