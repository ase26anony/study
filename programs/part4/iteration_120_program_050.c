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
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int common = 0;
    asm volatile("" : : : "memory");
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop */
        dummy1(i);
        
        /* Branch creates multiple basic blocks in outer loop */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            volatile int j = 0;
            
            /* Loop header - part of outer loop's true branch */
            dummy2(j);
            
            /* The loop body spans multiple blocks */
            for (j = 0; j < M1; ++j) {
                /* Body block A1 - inside inner loop A */
                dummy3(j);
                
                /* Shared block - will be jumped to from inside loop */
                if (j == M1/2) {
                    /* This goto creates partial overlap */
                    /* It jumps to a block in outer loop but outside the if branch */
                    goto shared_block;
                }
                
                /* Body block A2 - inside inner loop A */
                dummy4(j);
            }
            
            /* Continuation after loop A (inside if branch) */
            checksum += 1;
        } else {
            /* Inner Loop B - different from A but shares common prologue */
            volatile int k = 0;
            dummy2(k + 1000);  /* Different argument to prevent block merging */
            
            for (k = 0; k < M2; ++k) {
                /* Different dummy calls to create unique blocks */
                dummy1(k * 2);
                dummy3(k * 3);
            }
            
            checksum += 2;
        }
        
        /* Shared block - part of outer loop, reachable from inner loop A via goto */
        shared_block:
        dummy1(999);
        
        /* Another basic block in outer loop */
        asm volatile("" : : : "memory");
    }
    
    /* Memory barrier between loops */
    asm volatile("" : : : "memory");
    
    /* Sibling Loop C - shares common prologue but different body */
    /* This creates partial overlap with outer loop's blocks */
    {
        volatile int p = 0;
        /* Reuse common prologue block */
        common = 1;
        dummy2(common);
        
        for (p = 0; p < M3; ++p) {
            /* Different body from previous loops */
            dummy4(p * 4);
            dummy1(p * 5);
        }
        
        checksum += p;
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
