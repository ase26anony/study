/* Test program for hw-doloop.cc partial overlap bitmap analysis */
#include <stdio.h>
#include <stdlib.h>

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

int main(void) {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* Common prologue block shared by inner loops */
        volatile int shared = i * 2;
        dummy1(shared);
        
        /* Branch creating two paths through outer loop */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            volatile int j = 0;
            
            /* Label for goto to create partial overlap */
            loop_a_start:
            for (; j < M; ++j) {
                /* Unique body for loop A */
                dummy2(j);
                checksum += j;
                
                /* Jump to shared block outside if branch */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* Block only in true branch after loop A */
            dummy3(123);
            checksum += 123;
            
            /* Skip else branch */
            goto after_if;
        } else {
            /* Inner Loop B - distinct from loop A */
            for (volatile int k = 0; k < K; ++k) {
                /* Unique body for loop B */
                dummy3(k);
                checksum -= k;
            }
            
            /* Block only in else branch */
            dummy4(456);
            checksum += 456;
        }
        
        /* Shared block that both loops can reach */
        shared_block:
        dummy1(999);
        checksum += 999;
        
        after_if:
        /* Continue outer loop */
        asm volatile("" : : : "memory");
    }
    
    /* Sibling Loop C - shares prologue with inner loops but different body */
    /* Re-initialize shared variable to match pattern */
    volatile int shared = 0;
    dummy1(shared);
    
    for (volatile int c = 0; c < N/2; ++c) {
        /* Different body from loops A and B */
        dummy4(c);
        checksum += c * 3;
        
        /* Prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Another sibling-like structure with partial overlap */
    volatile int outer_shared = 0;
    dummy1(outer_shared);
    
    /* Loop D that partially overlaps with outer loop's blocks */
    for (volatile int d = 0; d < M; ++d) {
        /* This block is similar to outer loop's shared_block */
        if (d % 2 == 0) {
            dummy1(888);
            checksum += 888;
            goto partial_overlap_shared;
        } else {
            dummy2(777);
            checksum += 777;
        }
        
        partial_overlap_shared:
        /* This block exists in both loop D and outer loop */
        dummy3(666);
        checksum += 666;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force use of goto label to prevent dead code elimination */
    if (checksum > 1000000) {
        /* This should never execute, but keeps loop_a_start live */
        goto loop_a_start;
    }
    
    return 0;
}
