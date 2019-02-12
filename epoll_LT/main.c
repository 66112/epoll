#include <stdio.h>
#include <unistd.h>
int main()
{
    while(1){
        int addr[10] = {0};
        printf("%d \n",addr[1]);
        sleep(2);
        addr[1] = 1;
        printf("%d \n",addr[1]);
    }
    return 0;
}
