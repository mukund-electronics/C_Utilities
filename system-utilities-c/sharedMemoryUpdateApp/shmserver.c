
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 

int main(void)
{
    // ftok to generate unique key
    key_t key = ftok("shmfile", 65);

    // shmget returns an identifier in shmid
    int shmid = shmget(key, 1024, 0666|IPC_CREAT);

    int *count = (int*) shmat(shmid,(void*)0,0);

    for (*count = 0; *count <= 102400; (*count)++)
    {
        printf("%d\n", *count);
        sleep(1);
    }
    return 0;
}
