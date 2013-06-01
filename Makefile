CXX = g++
CXXFLAGS = -std=gnu++11 -g -O3 -Wall -pthread
LDFLAGS = -pthread -lboost_system -lboost_program_options
SRC = BoostSystemUtil.cpp \
      IoServicePool.cpp \
      IoThread.cpp \
      Log.cpp \
      Main.cpp \
      ProxyOptions.cpp \
      TcpProxy.cpp \
      TcpProxyClientAcceptor.cpp \
      TcpProxySession.cpp \
      TcpResolver.cpp \
      ThreadName.cpp
OBJS = ${SRC:.cpp=.o}

all: proxy

clean:
	rm -f *.o proxy

proxy: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

depend:
	$(CXX) $(CXXFLAGS) -MM $(SRC) > .makeinclude

include .makeinclude
