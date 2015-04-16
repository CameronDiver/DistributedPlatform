CC=../../../dcc

CFLAGS=-Wall

OBJS=true.o ../../../stdlib/syscall.o ../../../stdlib/stdlib.o

EXE=../bin/true

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
