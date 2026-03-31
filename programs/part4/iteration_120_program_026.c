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
    volatile int L = 75;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* COMMON PROLOGUE BLOCK - shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop only */
        dummy2(i);
        
        /* Branch creates multiple basic blocks within outer loop */
        if (cond) {
            /* Branch-specific block */
            dummy3(i);
            
            /* INNER LOOP A - starts inside if branch */
            volatile int j = 0;
            /* Label for goto to create partial overlap */
            loop_a_start:
            for (; j < M; ++j) {
                /* Loop A body - unique basic block */
                dummy4(j);
                checksum += j;
                
                /* This goto creates partial overlap by jumping
                   to a block that's in outer loop but outside if branch */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* This block is only in if branch, not in inner loop A */
            dummy1(i * 2);
        } else {
            /* INNER LOOP B - different branch, shares prologue */
            for (volatile int k = 0; k < K; ++k) {
                /* Loop B body - different from loop A */
                dummy2(k);
                checksum -= k;
            }
        }
        
        /* SHARED BLOCK - reached by goto from inner loop A
           This block is in outer loop but outside the if branch */
        shared_block:
        dummy3(checksum);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING LOOP C - shares prologue but different body */
    /* Re-initialize shared prologue */
    shared_counter = 0;
    dummy1(shared_counter);
    
    for (volatile int l = 0; l < L; ++l) {
        /* Different body from loops A and B */
        dummy4(l * 3);
        checksum += l * 2;
        
        /* Complex enough to create distinct basic blocks */
        if (l % 2) {
            dummy1(l);
        } else {
            dummy2(l);
        }
    }
    
    /* Additional complexity to create more partial overlaps */
    volatile int extra = 0;
    for (volatile int x = 0; x < 10; ++x) {
        /* This loop shares some setup with sibling loop C */
        dummy1(extra);
        
        /* Nested mini-loop inside */
        for (volatile int y = 0; y < 5; ++y) {
            dummy3(x + y);
            if (y == 2) {
                /* Jump creates partial overlap */
                goto partial_overlap;
            }
        }
        
        partial_overlap:
        dummy4(x);
        extra += x;
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
