#variables
#compilation flags
CC := gcc
CFLAGS := -Wall -Wextra -MMD -MP -Isrc
#Directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

#Output Executables
SERVER_BIN := $(BIN_DIR)/tftp_server
CLIENT_BIN := $(BIN_DIR)/tftp_client
BINS := $(SERVER_BIN) $(CLIENT_BIN)

#source (server , client , commons)
COMMON_SRCS := $(wildcard $(SRC_DIR)/commons/*.c)
SERVER_SRCS := $(wildcard $(SRC_DIR)/server/*.c) $(COMMON_SRCS)
CLIENT_SRCS := $(wildcard $(SRC_DIR)/client/*.c) $(COMMON_SRCS)

#objects
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

#Required Directories
REQUIRED_DIRS := $(BIN_DIR) $(OBJ_DIR)/server $(OBJ_DIR)/client $(OBJ_DIR)/commons
.PHONY : all clean debug client

all : $(BINS)

client : CFLAGS += -g
client : $(CLIENT_BIN)

debug : CFLAGS += -g
debug : all

$(SERVER_BIN) : $(SERVER_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(CLIENT_BIN) : $(CLIENT_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(REQUIRED_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(REQUIRED_DIRS) :
	mkdir -p $@

clean :
	rm -rf $(BUILD_DIR)

-include $(OBJ_DIR)/*/*.d
