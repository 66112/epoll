#include "epoll_server.hpp" 
#include <stdlib.h>
int main(int argc,char* argv[])
{
    if(argc != 2){
        std::cerr << "argument error!" << " port" << std::endl;
        exit(1);
    }
    Server* sp = new Server(atoi(argv[1]));
    if(!sp->InitServer()){
        return 1;
    }
    sp->StartServer();
    delete sp;
    return 0;
}
