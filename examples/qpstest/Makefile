SERVER := ServerDemo
CLIENT := ClientDemo

MUDUO_SRC := ../../simplifyMuduo/src/
MUDUO_INC := ../../simplifyMuduo/include/

RPC_SRC := ../../rpc/
RPC_INC := ../../rpc/

SUDOKU_INC := ./

COMMON_LIB := -lpthread -lprotobuf -lz -pthread

CXX = g++
CPPFLAGS = -g -O2 -Wall -std=c++11
CPPFLAGS += $(addprefix -I,$(MUDUO_INC))
CPPFLAGS += $(addprefix -I,$(RPC_INC))
CPPFLAGS += $(addprefix -I,$(SUDOKU_INC))

FILETYPE = cc
OBJS := $(MUDUO_SRC)*.$(FILETYPE)
OBJS += $(RPC_SRC)*.$(FILETYPE)

all : $(OBJS)
	$(CXX) $(CPPFLAGS) client.cc echo.pb.cc -o ClientDemo $(OBJS) $(COMMON_LIB)
	$(CXX) $(CPPFLAGS) server.cc echo.pb.cc -o ServerDemo $(OBJS) $(COMMON_LIB)

clean:
	-rm -f $(SERVER)
	-rm -f $(CLIENT)