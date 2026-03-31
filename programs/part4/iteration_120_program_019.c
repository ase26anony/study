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
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Outer Loop - contains multiple basic blocks */
    for (volatile int i = 0; i < N; ++i) {
        /* Common prologue block - shared by inner loops */
        volatile int shared = i * 2;
        dummy1(shared);
        
        /* Branch creates separate basic blocks */
        if (cond) {
            /* Inner Loop A - starts in true branch */
            /* This loop will partially overlap with outer loop */
            for (volatile int j = 0; j < M1; ++j) {
                dummy2(j);
                checksum += j;
                
                /* Jump to shared block outside the if branch */
                /* This creates partial overlap: inner loop contains
                   blocks from both inside and outside the if branch */
                if (j == M1/2) {
                    goto shared_block;
                }
            }
            
            /* Block only in true branch, after inner loop A */
            dummy3(123);
        } else {
            /* Inner Loop B - alternative inner loop */
            /* Shares the common prologue but has different body */
            for (volatile int k = 0; k < M2; ++k) {
                dummy4(k);
                checksum -= k;
                asm volatile("" : : : "memory");
            }
        }
        
        /* Shared block - part of outer loop but outside specific branches */
        shared_block:
        asm volatile("" : : : "memory");
        checksum += i;
        
        /* Reset condition to alternate branches */
        cond = !cond;
    }
    
    /* Sibling Loop C - shares some blocks with outer loop setup */
    /* Uses same dummy1 function as prologue, creating partial overlap */
    {
        /* Replicate the prologue pattern from outer loop */
        volatile int shared = 999;
        dummy1(shared);
        
        /* Different loop body */
        for (volatile int l = 0; l < M3; ++l) {
            dummy3(l * 3);
            checksum += l * 2;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
