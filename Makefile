INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CC = gcc
CFLAGS = -I $(INC_DIR) -lpthread

all: server client
	@echo "Build complete"

server:
	@gcc $(SRC_DIR)/sorter_server.c -o $(OBJ_DIR)/server $(CFLAGS)
	@ln -s ../$(OBJ_DIR)/server bin/server

client:
	@gcc $(SRC_DIR)/sorter_client.c -o $(OBJ_DIR)/client $(CFLAGS)
	@ln -s ../$(OBJ_DIR)/client bin/client

clean:
	@rm -rf obj/*
	@rm bin/server bin/client
	echo "Build cleaned"
