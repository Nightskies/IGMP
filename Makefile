BIN_NAME_CLIENT := client
BIN_PATH := bin

BUILD_PATH := build
BUILD_PATH_CLIENT := $(BUILD_PATH)

SRC_PATH := src

COMPILE_FLAGS := -Werror -g

SRC_EXT := c
CC := gcc

.PHONY: dirs_client clean client 

all: client

SRC_FILES_CLIENT := $(SRC_PATH)
SRC_FILES_CLIENT := $(wildcard $(addsuffix /*.$(SRC_EXT), $(SRC_FILES_CLIENT)))
SRC_OBJECTS_CLIENT := $(addprefix $(BUILD_PATH_CLIENT)/, $(notdir $(SRC_FILES_CLIENT:.$(SRC_EXT)=.o)))

client: dirs_client $(BIN_PATH)/$(BIN_NAME_CLIENT)

$(BIN_PATH)/$(BIN_NAME_CLIENT): $(SRC_OBJECTS_CLIENT)
	$(CC) $(COMPILE_FLAGS) $^ -o $@

VPATH := $(SRC_PATH)
$(BUILD_PATH_CLIENT)/%.o: %.$(SRC_EXT)
	$(CC) $(COMPILE_FLAGS) $^ -c -o $@

dirs_client:
	@mkdir $(BUILD_PATH_CLIENT) -p
	@mkdir $(BIN_PATH) -p

clean:
	@rm -rf $(BIN_PATH)
	@rm -rf $(BUILD_PATH)
