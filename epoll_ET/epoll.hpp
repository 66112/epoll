#pragma once 
/*
 * 1.当epoll检测到事件就绪时,必须立即处理,如果数据没读完,第二次调用wait后就不会再返回该事件了
 * 2.也就是说,ET模式下,fd下的时间就绪后,只有一次处理机会
 * 3.ET的性能就比较高了,epoll_wait返回的次数少了很多
 * 4.参数 struct epoll* 是已分配好的就绪文件描述符,直接用即可
 * 原理:
 * 1.调用 epoll_create 时,内核会创建一个红黑树,用来存放 epoll_ctl 添加进来的事件
 * 2.而所有添加到epoll的事件都会与网卡驱动程序建立回调关系,也就是说,当事件就绪时,会调用这个方法
 * 3.这个回调方法会将就绪的事件添加到 就绪链表中
 * 4.调用 epoll_wait 时,只需检查就绪链表中是否有元素即可
 * 5.如果就绪队列不为空,则把发生的事件复制到用户态,同时将时间数量返回给用户*/
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
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
    void Add_fd(int fd_,int op = EPOLLIN | EPOLLET)
    {
        Set_NoBlock(fd_);
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
    size_t NoBlockRead(int fd,char* buf,size_t size) //非阻塞式读
    {
        size_t total_size = 0;  //已读的大小
        for(; ;){
            size_t cur_size = read(fd, buf + total_size, size);
            if(cur_size == 0) return 0;
            total_size += cur_size;
            if(cur_size < size || errno == EAGAIN || total_size == size){
                break;
            }
        }
        buf[total_size] = 0;
        return total_size;
    }
private:
    void Set_NoBlock(int fd)
    {
        int old_option = fcntl(fd,F_GETFL);  //获得文件的旧状态
        if(old_option < 0){
            std::cerr << "fcntl failure!" << std::endl;
            return;
        }
        fcntl(fd, F_SETFL, old_option | O_NONBLOCK); //设置文件的新状态 
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
