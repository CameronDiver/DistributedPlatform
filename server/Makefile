CC=clang++

CPPFLAGS=-Wall -std=c++11
LFLAGS=-ldl

OBJS=process.o main.o util.o

EXE=main

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CPPFLAGS) $(LFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
		rm $(OBJS)
