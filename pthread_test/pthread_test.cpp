#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void *mypthreadFunction(void *pvoid)
{
        int *i = (int*)pvoid;  // 危险！！！线程里用指针指向main函数里变量的地址，后续改写操作会影响到main函数该地址上的值
        int j = 0;

        // 此时i指向的main函数的栈帧
        printf("thread i addr: %p, val: %d\n", i, *i);

        while(j < 10)
        {
                printf("thread function, i: %d\n", (*i)++);
                j++;
                sleep(1);
        }
}

int main()
{
        int i = 10;
        int j = 0;
        pthread_t tid;

        printf("-main- i addr: %p, val: %d\n", &i, i);
        pthread_create(&tid, NULL, mypthreadFunction, (void *)&i); // 传局部自动变量地址

        while(j < 6)
        {
                printf("-main- function, i: %d\n", i);
                j++;
                sleep(1);
        }
        printf("-main- function exit!\n");
        pthread_exit(NULL);
    return(0);
}
