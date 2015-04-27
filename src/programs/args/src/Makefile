CC=../../../dcc

CFLAGS=-Wall

OBJS=args.o

EXE=../bin/args

ALL: $(EXE)

$(EXE): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
		@rm -f $(OBJS)
