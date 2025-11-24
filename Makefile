# NDM_TZ Makefile
CXX      =  g++
CXXFLAGS = -std=c++11
LDFLAGS  = #
 
all: compile

compile:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) ./src/epoll_server.cpp -o ./bin/epoll_server $(LDFLAGS)

# delete build directory
clean: 
	@rm -rf ./bin
#

.PHONY: all compile clean