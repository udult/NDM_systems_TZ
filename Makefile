# NDM_TZ Makefile
CXX      =  g++
CXXFLAGS = -std=c++11
LDFLAGS  =  #
 
all: compile

compile:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) ./src/epoll_server.cpp -o ./bin/epoll_server $(LDFLAGS)

install:
	sudo mkdir -p /usr/local/bin
	sudo cp ./bin/epoll_server /usr/local/bin/epoll_server
	sudo cp epoll_server.service /etc/systemd/system/epoll_server.service

# delete build directory
clean: 
	@rm -rf ./bin
#

.PHONY: all install clean
