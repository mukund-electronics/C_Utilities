#include<stdio.h>

typedef void (*fun_ptr)();

void fun1(){
    printf("fun1\n");
}
void fun2(){
    printf("fun2\n");
}
void fun3(){
    printf("fun3\n");
}
void fun4(){
    printf("fun4\n");
}
void fun5(){
    printf("fun5\n");
}

void callfunptr(fun_ptr fn){
    fn();
}

const void *arr[5] = {fun1,fun2,fun3,fun4,fun5};

void main(){
    for(int i = 0; i < 5; i++){
        callfunptr(arr[i]);
    }
    
    for(int i = 0; i < 5; i++){
        fun_ptr funcall = arr[i];
        funcall();
    }
    
   // for(int i = 0; i < 5; i++){
   //    (*(int *)arr[i])();
   // }
}
