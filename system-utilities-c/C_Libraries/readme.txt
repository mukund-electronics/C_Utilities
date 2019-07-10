/*Platform portability*/
dataTypes.h
errorTypes.h

/*Platform specifics*/
platform.h

/*Platform oriented Timer Abstraction*/
platformTimer.c
platformTimer.h

/*Configurable Timer Framework - supports Realtime, High Priority and Low Priority timers*/
timerFw.c
timerFw.h

/*Static Queue Library*/
staticQueue.h
staticQueue.c

/*Event Framework - dependency: Static Queue Library*/
eventFw.h
eventFw.c

/*Test Files*/
timerFwTest.c
staticQueueTest.c
eventFwTest.c


/*Build Commands*/
gcc -g -o tfwTest timerFwTest.c timerFw.c platformTimer.c
gcc -g -o staticQueueTest staticQueueTest.c staticQueue.c
gcc -g -o eventFwTest eventFwTest.c eventFw.c staticQueue.c
