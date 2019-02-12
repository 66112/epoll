.PHONY:all
cc = g++
all:epoll_server epoll_client 
epoll_server:epoll.hpp epoll_server.hpp Tcp_Server.cc 
	$(cc) -std=c++11 -o  $@ $^ -g
epoll_client:Tcp_Client.cc
	$(cc) -o $@ $^
.PHONY:clean
clean:
	rm -rf epoll_server epoll_client

