CC = clang
ARGS = -Wall

all: server

linked_list.o: linked_list.c linked_list.h
	$(CC) -c linked_list.c $(ARGS)

server: server.c linked_list.o
	$(CC) -o server server.c linked_list.o  $(ARGS) -lpthread -g

clean: 
	rm -rf server *.o

