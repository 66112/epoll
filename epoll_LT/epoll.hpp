/*
 *1.当epoll检测到fd下的事件就绪以后,可以不立即处理,或只处理一部分
 *2.如果缓冲区中的数据没有处理完,
 *则第二次调用 epoll_wait 的时候可以继续处理,直到数据处理完为止
 *  
 *  */
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <map>
#define EPOLL_SIZE 64
class Epollor
{
public:
    Epollor()
    {}
    ~Epollor()
    {}
    void Create_epfd(size_t size = EPOLL_SIZE)
    {
        _ep_fd = epoll_create(size);
        if(_ep_fd < 0){
            std::cout << "epoll_create failure!" << std::endl;
        }
    }
    void Add_fd(int fd_,int op = EPOLLIN)
    {
        struct epoll_event ev;    
        ev.data.fd = fd_;
        ev.events = op;
        int ret = epoll_ctl(_ep_fd,EPOLL_CTL_ADD,fd_,&ev);
        if(ret < 0){
            std::cerr << "Add fd failure!" << std::endl;
        }
    }
    void Insert_cli_msg(int fd,sockaddr_in& client)
    {
        _cli_msg.insert(std::pair<int,sockaddr_in>(fd,client));
    }
    void Find_cli_msg(int fd,sockaddr_in& cli)
    {
        auto it = _cli_msg.find(fd);
        cli = it -> second;
    }
    void Del_fd(int fd)
    {
        epoll_ctl(_ep_fd,EPOLL_CTL_DEL,fd,NULL);
        auto it = _cli_msg.find(fd);
        _cli_msg.erase(it);
    }
    void Modif(int fd_,int op)     //修改
    {
        struct epoll_event ev;    
        ev.data.fd = fd_;
        ev.events = op;
        int ret = epoll_ctl(_ep_fd,EPOLL_CTL_MOD,fd_,&ev);
        if(ret < 0){
            std::cerr << "MOD fd failure!" << std::endl;
        }
    }
    int Wait_Epoll(struct epoll_event* pev)
    {
        int ret;
        switch (ret = epoll_wait(_ep_fd,pev,EPOLL_SIZE,1000)){
            case -1:
                std::cerr << "epoll error!" << std::endl;
                break;
            case 0:
                std::cerr << "timeout!!!" << std::endl;
                break;
            default:
                return ret;
        }
        return ret - 1;
    }
public:
    bool Is_OK_rd(struct epoll_event& ev)
    {
        return ev.events & EPOLLIN;
    }
private:
    int _ep_fd;         //句柄
    std::map<int,sockaddr_in> _cli_msg;
};
