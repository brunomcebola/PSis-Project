CC = gcc
CFLAGS = -lpthread

local_server = KVS/KVS-LocalServer/local-server.c
local_server_console = KVS/KVS-LocalServer/Console/console-lib.c
local_server_connections = KVS/KVS-LocalServer/Connections/connections-lib.c
auth_server = KVS/KVS-AuthServer/auth-server.c
kvs_lib = KVS/KVS-Lib/KVS-lib.c
hashtable_lib = Hashtable/hashtable-lib.c
test_file = Applications/test.c
personalized_test = Applications/personalized_app.c
configs = KVS/configs.c

all: exesFolder localServer authServer test personalized

localServer: $(local_server) $(local_server_connections) $(local_server_console) $(hashtable_lib) $(configs)
	$(CC) $^ -g -o ./executables/$@ $(CFLAGS) 

authServer: $(auth_server) $(hashtable_lib) $(configs)
	$(CC) $^ -g -o ./executables/$@ $(CFLAGS)

test: $(test_file) $(kvs_lib) $(configs)
	$(CC) $^ -o ./executables/$@ $(CFLAGS)

personalized: $(personalized_test) $(kvs_lib) $(configs)
	$(CC) $^ -o ./executables/$@ $(CFLAGS)

clean:
	rm -r executables

exesFolder:
	mkdir -p executables

.PHONY: all clean