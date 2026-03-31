/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop */

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
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int sum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int shared = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop */
        dummy1(i);
        
        /* Complex if-else structure creates multiple basic blocks */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            shared = i * 2;
            
            /* Loop A body - will jump to shared block */
            for (volatile int j = 0; j < M; ++j) {
                dummy2(j);
                sum += j;
                
                /* Jump to block outside if-else but still in outer loop */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* Block only executed if no goto taken */
            dummy3(i);
        } else {
            /* Inner Loop B - different from Loop A */
            shared = i * 3;
            for (volatile int k = 0; k < K; ++k) {
                dummy4(k);
                sum -= k;
            }
        }
        
shared_block:
        /* This block is in outer loop, executed by:
           - Normal flow from if/else
           - goto from Inner Loop A
        */
        asm volatile("" : : : "memory");
        sum += shared;
    }
    
    /* Sibling Loop C - shares prologue with inner loops */
    /* This creates partial overlap with outer loop */
    shared = 0;
    for (volatile int c = 0; c < N/2; ++c) {
        /* Same prologue pattern as inner loops */
        volatile int temp = shared + c;
        dummy1(temp);
        
        /* Different body from other loops */
        for (volatile int d = 0; d < 10; ++d) {
            dummy2(d);
            sum += d * c;
        }
        
        asm volatile("" : : : "memory");
    }
    
    /* Another overlapping structure for more complex bitmap analysis */
    {
        volatile int x = 0;
        volatile int y = 0;
        
        /* Loop D and E will partially overlap */
        for (x = 0; x < 200; x++) {
            dummy3(x);
            if (x % 3 == 0) {
                /* Loop E inside D but extends beyond */
                for (y = 0; y < 100; y++) {
                    dummy4(y);
                    if (y == 50) {
                        /* Jump to block in D but outside E's normal scope */
                        goto middle_of_D;
                    }
                }
                /* Block in D, only if no goto */
                sum += x;
            }
middle_of_D:
            /* Shared block between D and E */
            asm volatile("" : : : "memory");
            sum += x * 2;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
