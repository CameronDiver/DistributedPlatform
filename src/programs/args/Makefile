CC=../../dcc

CFLAGS=-Wall

OBJS=args.o ../../stdlib/syscall.o ../../stdlib/stdio.o ../../stdlib/stdlib.o ../../stdlib/sys.o

EXE=args.so

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
		@rm -f $(OBJS)
