# NDM_TZ Makefile
CXX      =  g++
CXXFLAGS = -std=c++11
LDFLAGS  = #
 
all: compile

compile:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) ./src/epoll_server.cpp -o ./bin/epoll_server $(LDFLAGS)

install:
	sudo cp ./bin/epoll_server /usr/local/bin/
	sudo cp epoll_server.service /etc/systemd/system/

# delete build directory
clean: 
	@rm -rf ./bin
#

.PHONY: all compile clean
