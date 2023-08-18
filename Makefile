
CC1 = g++ -Wno-write-strings
CC2 = gcc
SERVER_FILE = simple_server.cpp
HTTP_SERVER_FILE = http_server.cpp
LOAD_GEN_FILE = load_gen.c

all: server load_gen

server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC1) $(SERVER_FILE) $(HTTP_SERVER_FILE) -o server
	
load_gen: $(LOAD_GEN_FILE)
	$(CC2) $(LOAD_GEN_FILE) -lpthread -o load_gen

clean:
	rm -f server load_gen server *.log
