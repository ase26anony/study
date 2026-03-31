/* Test program for hw-doloop.cc partial overlap bitmap analysis */
#include <stdio.h>

/* Dummy functions to create unique basic blocks */
void __attribute__((noinline, noclone)) dummy1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main() {
    /* Volatile variables to prevent optimization */
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int L = 200;
    volatile int counter = 0;
    volatile int flag = 1;
    
    /* COMMON PROLOGUE BLOCK - shared by multiple loops */
    volatile int shared_init = 0;
    dummy1(shared_init);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        asm volatile("" : : : "memory");
        
        /* Complex if-else structure creates multiple basic blocks */
        if (flag) {
            /* Branch 1 */
            dummy1(i);
            
            /* INNER LOOP A - starts inside if branch */
            volatile int j = 0;
            
            /* Loop A prologue - unique to this loop */
            dummy2(j);
            
shared_label: /* SHARED BLOCK - jumped to from inside inner loop */
            /* This block is part of both outer loop and inner loop A */
            dummy3(j);
            
            /* Inner Loop A body */
            for (; j < M; ++j) {
                asm volatile("" : : : "memory");
                counter += j;
                dummy2(j);
                
                /* Jump to shared block that's outside the if branch
                   but still within outer loop */
                if (j == M/2) {
                    goto shared_label;
                }
            }
            
            /* Continuation after inner loop A */
            dummy4(i);
        } else {
            /* Branch 2 */
            dummy2(i);
            
            /* INNER LOOP B - shares common prologue but different body */
            /* Re-initialize shared_init to create common block pattern */
            shared_init = i;
            dummy1(shared_init);
            
            for (volatile int k = 0; k < K; ++k) {
                asm volatile("" : : : "memory");
                counter -= k;
                dummy3(k);
            }
        }
        
        /* Outer loop continuation block - still part of outer loop */
        dummy4(counter);
    }
    
    /* SIBLING LOOP C - shares common prologue with inner loops
       but has different body and iteration count */
    /* Re-use the common prologue block pattern */
    shared_init = 999;
    dummy1(shared_init);
    
    for (volatile int l = 0; l < L; ++l) {
        asm volatile("" : : : "memory");
        counter += l * 2;
        dummy4(l);
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", counter);
    
    return 0;
}
