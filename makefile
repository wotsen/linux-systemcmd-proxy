CC=g++

DEBUG = false

MAKE_INSTALL_PREFIX = .

NET_SYSTEM = systemcmd_proxyd
OBJ_STATIC_LIB = libsystemcmd_proxy_cli.a

SRV_SRC = ./src/systemcmd_proxy_service.cpp
CLI_SRC = ./src/systemcmd_proxy_interface.cpp

SRV_OBJ = $(patsubst %cpp, %o, $(SRV_SRC))
CLI_OBJ = $(patsubst %cpp, %o, $(CLI_SRC))

LIBS = -lsystemcmd_proxy_cli
INC = -I./src/

INCS = ./src/systemcmd_proxy_interface.h

CFLAGS = -lpthread -W -O3

ifeq ($(DEBUG), true)
CFLAGS += -DDEBUG
endif

LOC_INC = $(MAKE_INSTALL_PREFIX)/include/
LOC_BIN = $(MAKE_INSTALL_PREFIX)/bin/
LOC_LIB = $(MAKE_INSTALL_PREFIX)/lib/

all:$(NET_SYSTEM) $(OBJ_STATIC_LIB)

$(NET_SYSTEM):$(SRV_OBJ)
	$(CC) $^ -o $@ $(INC) $(CFLAGS)

$(OBJ_STATIC_LIB):$(CLI_OBJ)
	ar -r $@ $<

%.o:%.cpp
	$(CC) $< -c -o $@ $(INC) $(CFLAGS)

.PHONY: install
install:
	-mkdir -p $(LOC_INC)
	-cp $(INCS) $(LOC_INC) -f
	-mkdir -p $(LOC_BIN)
	-cp $(NET_SYSTEM) $(LOC_BIN) -f
	-mkdir -p $(LOC_LIB)
	-cp $(OBJ_STATIC_LIB) $(LOC_LIB) -f
cli_test:
	$(CC) ./test/main.cpp -o $@ -I$(LOC_INC) $(LIBS) -L$(LOC_LIB)

clean:
	rm -rf $(SRV_OBJ) $(CLI_OBJ) $(OBJ_STATIC_LIB) $(NET_SYSTEM) cli_test $(LOC_INC) $(LOC_BIN) $(LOC_LIB)
