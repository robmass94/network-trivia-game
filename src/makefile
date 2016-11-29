CC=clang++ -std=c++11

all: server client

##################################################### CHAT SERVER #####################################################
server: server.o shared.o stringutils.o triviabot.o
	$(CC) -pthread -o server server.o shared.o stringutils.o triviabot.o

server.o:
	$(CC) -c server.cpp

##################################################### CHAT CLIENT #####################################################
client: client.o shared.o stringutils.o
	$(CC) -lncurses -pthread -o client client.o shared.o stringutils.o

client.o:
	$(CC) -c client.cpp

#################################################### SHARED STUFF #####################################################
stringutils.o: stringutils.cpp
	$(CC) -c stringutils.cpp

shared.o: shared.cpp
	$(CC) -c shared.cpp

triviabot.o: triviabot.cpp
	$(CC) -c triviabot.cpp

####################################################### COMMANDS ######################################################
clean:
	rm -f client server *.o

