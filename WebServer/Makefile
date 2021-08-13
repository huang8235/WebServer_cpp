CC:= g++
CFLAGS:= -g -Wall -std=c++11 -g
LIBS:= -lpthread

TARGET = MyServer
SOURCE := $(wildcard *.cpp base/*.cpp)
BASE := $(basename $(SOURCE))
OBJS := $(addsuffix .o, $(addprefix obj/, $(BASE)))

.PHONY: all objs clean
all: $(TARGET)
objs: $(OBJS)
clean:
	-rm -f $(TARGET)
	-rm -f obj/*.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

obj/%.o: %.cpp %.cpp
	$(CC) -c -o $@ $<

#obj/base/%.o: base/%.cpp
#	$(CC) -c -o $@ $<
