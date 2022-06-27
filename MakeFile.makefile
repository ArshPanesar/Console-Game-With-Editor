CC = gcc
CFLAGS = -g

SRC_DIR := src
BUILD_DIR := intermediates

SOURCES := $(wildcard $(SRC_DIR)/*.c) #$(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
					$(CC) $(CFLAGS) -c $< -o $@

TARGET := CGame

$(TARGET) : $(OBJECTS)
			$(CC) $(OBJECTS) -o $(TARGET)

.PHONY : clean
clean :
		rm $(OBJECTS)