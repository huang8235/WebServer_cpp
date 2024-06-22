CC:= g++
CFLAGS:= -g -Wall -std=c++11 -g
LIBS:= -lpthread

OBJ_DIR := ./temp/obj
TARGET_DIR := ./app
TARGET = $(TARGET_DIR)/MyServer

SOURCE := $(wildcard *.cpp base/*.cpp)
BASE := $(basename $(SOURCE))
OBJS := $(addsuffix .o, $(addprefix $(OBJ_DIR)/, $(BASE)))

.PHONY: all objs clean
all: $(TARGET)
objs: $(OBJS)
clean:
	-rm -f $(TARGET)
	-rm -f $(OBJ_DIR)/*.o

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $<
