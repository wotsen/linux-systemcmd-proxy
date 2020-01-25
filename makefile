CC=g++

NET_SYSTEM = net_system_cmd_proxyd
OBJ_STATIC_LIB = libnetsys_cmd_proxy_cli.a

SRV_SRC = ./src/systemcmd-service.cpp
CLI_SRC = ./src/netsystemcmd-proxy-interface.cpp

SRV_OBJ = $(patsubst %cpp, %o, $(SRV_SRC))
CLI_OBJ = $(patsubst %cpp, %o, $(CLI_SRC))

LIBS = -lnetsys_cmd_proxy_cli
INC = -I./src/

INCS = ./src/netsystemcmd-proxy-interface.h

CFLAGS = -W -O3

LOC_INC = ./include/
LOC_BIN = ./bin/
LOC_LIB = ./lib/

all:$(NET_SYSTEM) $(OBJ_STATIC_LIB)
	-mkdir -p $(LOC_INC)
	-cp $(INCS) $(LOC_INC) -f
	-mkdir -p $(LOC_BIN)
	-cp $(NET_SYSTEM) $(LOC_BIN) -f
	-mkdir -p $(LOC_LIB)
	-cp $(OBJ_STATIC_LIB) $(LOC_LIB) -f

$(NET_SYSTEM):$(SRV_OBJ)
	$(CC) $^ -o $@ $(INC) $(CFLAGS)

$(OBJ_STATIC_LIB):$(CLI_OBJ)
	ar -r $@ $<

%.o:%.cpp
	$(CC) $< -c -o $@ $(INC) $(CFLAGS)

cli_test:
	$(CC) ./test/main.cpp -o $@ -I$(LOC_INC) $(LIBS) -L$(LOC_LIB)

clean:
	rm -rf $(SRV_OBJ) $(CLI_OBJ) $(OBJ_STATIC_LIB) $(NET_SYSTEM) cli_test $(LOC_INC) $(LOC_BIN) $(LOC_LIB)
