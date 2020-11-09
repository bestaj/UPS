all: clean server

server: server.c client.c message.c room.c
	gcc $^ -o server

clean:
	rm -f server

