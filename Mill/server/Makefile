all: clean server

server: server.c client.c message.c room.c
	gcc -pthread $^ -o server

clean:
	rm -f server

