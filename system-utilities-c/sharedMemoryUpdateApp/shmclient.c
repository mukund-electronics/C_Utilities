#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
 
int main()
{
    // ftok to generate unique key
    key_t key = ftok("shmfile",65);
 
    // shmget returns an identifier in shmid
    int shmid = shmget(key,1024,0666|IPC_CREAT);
 
    // shmat to attach to shared memory
    int *count = (int*) shmat(shmid,(void*)0,0);
 
    printf("%d\n", *count);
     
    //detach from shared memory 
    shmdt(count);
    return 0;
}
