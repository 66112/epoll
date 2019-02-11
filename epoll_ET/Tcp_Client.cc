#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc,char* argv[])
{
    if(argc != 3){
        std::cerr << "argument error!" << std::endl;
        exit(1);
    }
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
        std::cout << "socket error!" << std::endl;
        exit(2);
    }    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(sock,(struct sockaddr*)&server,sizeof(server));
    if(ret < 0){
        std::cerr << "connect error!" << std::endl;
        exit(3);
    }
    std::string message;
    char buf[1024];
    for( ; ; ){
        std::cout << "Please Enter# ";
        std::cin >> message;
        write(sock,message.c_str(),message.size());
        ssize_t sz = read(sock,buf,sizeof(buf)-1);
        if(sz > 0){
            buf[sz] = 0;
            std::cout << "server# " << buf << std::endl;
        }else if(sz == 0){
            std::cout << "server error!" << std::endl;
            break;
        }
        else{
            std::cout << "read error!" << std::endl;
            continue;
        }
    }
    close(sock);
    return 0;
}
