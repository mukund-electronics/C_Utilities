#include<stdio.h>

typedef enum{
    event_1 = 0,
    event_2,
    event_3,
    event_4,
    event_5,
    event_Max
}event_t;

typedef event_t (*fun_ptr)();

event_t fun1(){
    printf("fun1\n");
}
event_t fun2(){
    printf("fun2\n");
}
event_t fun3(){
    printf("fun3\n");
}
event_t fun4(){
    printf("fun4\n");
}
event_t fun5(){
    printf("fun5\n");
}

void callfunptr(fun_ptr fn){
    fn();
}

const void *arr[event_Max] = {
     [event_1] = fun1,
     [event_2] = fun2,
     [event_3] = fun3,
     [event_4] = fun4,
     [event_5] = fun5
};

void main(){
    for(int i = 0; i < 5; i++){
        callfunptr(arr[i]);
    }
    
    callfunptr(arr[event_1]);
}
