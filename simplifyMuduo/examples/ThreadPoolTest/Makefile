TARGET := ThreadPoolTestDemo

SRC := ../../src/
INC := ../../include/
LIB := -lpthread

CXX = g++
CPPFLAGS = -g -O2 -Wall -std=c++11
CPPFLAGS += $(addprefix -I,$(INC))

FILETYPE = cc
OBJS := $(SRC)*.$(FILETYPE)
#OBJS := $(addsuffix .o, $(basename $(wildcard $(SRC)*.$(FILETYPE))))
#objs : $(OBJS)

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(OBJS) ThreadPoolTest.cc $(LIB)
#$(CXX) $(CPPFLAGS) -o $(TARGET) ThreadPoolTest.cc $(LIB)
clean:
	-rm -f $(SRC)*.o $(TARGET)
