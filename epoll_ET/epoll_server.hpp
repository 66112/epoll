#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <string.h>
#include "epoll.hpp"
using namespace std;
class Sock
{
public:
    Sock():_fd(-1)
    {}
    bool Socket()
    {
        _fd = socket(AF_INET,SOCK_STREAM,0);
        if(_fd < 0){
            std::cout << "socket error!" <<std::endl;
            return false;
        }
        return true;
    }
    bool Solve_TIME_WAIT()
    {
        int opt = 1;
        int ret = setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        if(ret < 0){
            std::cout << "setsockopt error!" << std::endl;
            return false;
        }
        return true;
    }
    bool Bind(const uint16_t& port_)
    {
        struct sockaddr_in local;
        bzero(&local,sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port_);
        local.sin_addr.s_addr = INADDR_ANY;
        int ret = bind(_fd,(struct sockaddr*)&local,sizeof(local));
        if(ret < 0){
            std::cout << "bind error!" << std::endl;
            return false;
        }
        return true;
    }
    bool Listen(int num)
    {
        int ret = listen(_fd,num);
        if(ret < 0){
            std::cout << "listen error!" << std::endl;
            return false;
        }
        return true;
    }
    int Accept(struct sockaddr_in* pclient = NULL,socklen_t* plen = NULL)
    {
        int new_sock = accept(_fd,(struct sockaddr*)pclient,plen); 
        if(new_sock < 0){
            std::cout << "accept error!" << std::endl;
            return -1;
        }
        std::cout << "New Client Connect!" << std::endl;
        return new_sock;
    }
    bool Recv(struct sockaddr_in& client)
    {
        char buf[1024] = {0};
        int ret = recv(_fd,buf,sizeof(buf)-1,0);
        if(ret < 0){
            std::cout << "recv error" << std::endl;
            return false;
        }
        else if(ret == 0){
            std::cout << "client quit!" << std::endl;
            return false;
        }
        std::cout << "ip: " << inet_ntoa(client.sin_addr) << " "\
            << ntohs(client.sin_port) << " # " << buf << std::endl;
        return true;
    }
    bool Send(int fd,char* msg)
    {
        send(fd,msg,strlen(msg),0);
        return true;
    }
    int GetListenfd()
    {
        return _fd;
    }
    void Close()
    {
        close(_fd);
    }
    ~Sock()
    {}
private:
    int _fd;
};
class Server
{
public:
    Server(const uint16_t port):_port(port)
    {}
    bool InitServer()
    {
        return _Listen_sock.Socket()
             &&_Listen_sock.Solve_TIME_WAIT()
             &&_Listen_sock.Bind(_port)
             &&_Listen_sock.Listen(5);
    }
    void StartServer()
    {
        int lis_fd = _Listen_sock.GetListenfd(); //拿到监听描述符
        Epollor epollor;
        epollor.Create_epfd();
        epollor.Add_fd(lis_fd);
        while(1){
            struct epoll_event ev[EPOLL_SIZE];
            int size = epollor.Wait_Epoll(ev);
            //cli_event是一个输出型参数,epoll会把发生的事件赋值到cli_event中
            if(size <= 0) continue;
            for(int i = 0; i < size; i++){
                if(epollor.Is_OK_rd(ev[i])){
                    sockaddr_in client;
                    socklen_t len = sizeof(client);
                    if(ev[i].data.fd == lis_fd){
                        int new_sock = _Listen_sock.Accept(&client,&len);
                        epollor.Add_fd(new_sock);
                        epollor.Insert_cli_msg(new_sock,client);
                    }
                    else{
                        char buf[1024];
                        int cli_fd = ev[i].data.fd;
                        int rd_num = epollor.NoBlockRead(cli_fd,buf,sizeof(buf)-1); 
                        epollor.Find_cli_msg(cli_fd,client);
                        if(rd_num <= 0){
                            close(cli_fd);
                            std::cerr << "client quit! "\
                            << "ip: "<< inet_ntoa(client.sin_addr)\
                            << " port: " << ntohs(client.sin_port) << std::endl;
                            epollor.Del_fd(cli_fd);
                        }
                        else{
                            std::cout << "ip: "<< inet_ntoa(client.sin_addr)\
                            << " port: " << ntohs(client.sin_port) \
                            << " client# " << buf << std::endl;
                            string msg = "i have a dream!!!";
                            write(cli_fd,msg.c_str(),msg.size());
                        }
                    }
                }
            }
        }
    }
    ~Server()
    {}
private:
    uint16_t _port;    
    Sock _Listen_sock;
};
