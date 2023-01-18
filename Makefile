DEBUG= 
EXECS= server client

all:	$(EXECS)

server:	server.c
	gcc $(DEBUG) -pthread -o server server.c

client:	client.c
	gcc $(DEBUG) -pthread -o client client.c

clean:
	rm -f client server
