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
CLIENT_SRCS := $(wildcard $(SRC_DIR)/client/*.c)