COMPILER=gcc

all:
	make checkpipe
	make run
	make server
	make client


checkpipe: ./checkpipe.c
	$(COMPILER) ./checkpipe.c -o checkpipe

run: ./run.c
	$(COMPILER) ./run.c -o run

server: ./server.c
	$(COMPILER) ./collection.c ./server.c -o argusd

client: ./client.c
	$(COMPILER) ./client.c -o argus


clean:
	rm argus argusd checkpipe run toClient toServer history log log.idx

