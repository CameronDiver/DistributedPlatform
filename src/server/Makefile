CC=clang++

CPPFLAGS=-Wall -std=c++11
LFLAGS=-ldl -lsqlite3

OBJS=devices.o fs.o fsdirect.o process.o main.o server.o util.o connection.o net/socket.o net/sockettcp.o log.o devices/device.o devices/devfull.o devices/devnull.o devices/devsocket.o devices/devzero.o

EXE=main

ALL: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CPPFLAGS) $(LFLAGS) $(OBJS) -o $(EXE)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
		rm -f $(OBJS)
