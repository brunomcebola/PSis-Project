CC = gcc
CFLAGS = -lpthread

local_server = KVS/KVS-LocalServer/local-server.c
local_server_console = KVS/KVS-LocalServer/Console/console-lib.c
local_server_connections = KVS/KVS-LocalServer/Connections/connections-lib.c
auth_server = KVS/KVS-AuthServer/auth-server.c
kvs_lib = KVS/KVS-Lib/KVS-lib.c
hashtable_lib = Hashtable/hashtable-lib.c
test_file = test.c

all: clean exesFolder localServer authServer test

localServer: $(local_server) $(local_server_connections) $(local_server_console) $(hashtable_lib)
	$(CC) $^ -o ./executables/$@ $(CFLAGS) 

authServer: $(auth_server) $(hashtable_lib)
	$(CC) $^ -o ./executables/$@ $(CFLAGS)

test: $(test_file) $(kvs_lib) $(hashtable_lib)
	$(CC) $^ -o ./executables/$@ $(CFLAGS)

clean:
	rm -r executables

exesFolder:
	mkdir -p executables

.PHONY: all clean