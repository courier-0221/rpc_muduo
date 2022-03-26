TARGET := RpcCodecDemo

MUDUO_SRC := ../../SimplifyMuduo/src/
MUDUO_INC := ../../SimplifyMuduo/include/

RPC_SRC := ../../rpc/
RPC_INC := ../../rpc/

COMMON_LIB := -lpthread -lprotobuf -lz -pthread

CXX = g++
CPPFLAGS = -g -O2 -Wall -std=c++11
CPPFLAGS += $(addprefix -I,$(MUDUO_INC))
CPPFLAGS += $(addprefix -I,$(RPC_INC))

FILETYPE = cc
OBJS := $(MUDUO_SRC)*.$(FILETYPE)
OBJS += $(RPC_SRC)*.$(FILETYPE)

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(CPPFLAGS) RpcCodec_test.cc -o $(TARGET) $(OBJS) $(COMMON_LIB)

clean:
	-rm -f $(TARGET)
