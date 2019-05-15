#include<stdio.h>


void pr(int a, int b){
    printf("sum : %d\n", a+b);
}

typedef void (*fun_ptr)(int,int);

void callfunptr(fun_ptr func, int a, int b){
    func(a, b);
}

void main(){
    int a = 5;
    int b = 6;
    fun_ptr fun2 = pr;
    
    callfunptr(pr, a, b);
    
    fun2(a,b);
}
