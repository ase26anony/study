/* Test program for hw-doloop.cc uncovered lines 429-436 */
/* Creates loops with partial block overlap to trigger bitmap intersection logic */

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
    /* Volatile variables to prevent optimization */
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int counter = 0;
    volatile int flag = 1;
    
    /* Outer Loop - will contain partial overlaps */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common prologue block - shared by inner loops */
        volatile int shared = outer * 2;
        dummy1(shared);
        
        /* Branch creates separate basic blocks */
        if (flag) {
            /* Inner Loop A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int inner_a = 0;
            
            /* Label for goto to create partial overlap */
        inner_loop_a_start:
            for (inner_a = 0; inner_a < M; ++inner_a) {
                /* Unique body for inner loop A */
                dummy2(inner_a + outer);
                counter++;
                
                /* Jump to shared block outside if branch */
                /* This creates partial overlap: inner loop contains blocks
                   both inside and outside the if branch */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            
            /* Continue after inner loop A */
            dummy3(outer * 3);
        } else {
            /* Inner Loop B - alternative path */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b + outer + 1000);
                counter--;
            }
        }
        
        /* Shared block - part of outer loop, also jumped to from inner loop A */
        /* This creates the partial overlap condition */
    shared_block:
        dummy4(outer + 999);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Sibling Loop C - shares prologue with outer loop but different body */
    /* This creates another partial overlap scenario */
    {
        /* Reuse similar prologue pattern */
        volatile int shared = N * 3;
        dummy1(shared);
        
        /* Different loop structure */
        for (volatile int sibling = 0; sibling < N/2; ++sibling) {
            /* Different body to avoid merging */
            dummy2(sibling * 5);
            dummy3(sibling + 777);
            counter += 2;
            
            /* Small inner loop to create more complex bitmap */
            for (volatile int inner_sib = 0; inner_sib < 10; ++inner_sib) {
                dummy4(inner_sib + sibling);
            }
        }
    }
    
    /* Another overlapping structure: Loop D that shares some blocks with outer */
    {
        volatile int setup = counter;
        dummy1(setup);
        
        /* Loop D overlaps with outer's prologue but has different body */
        for (volatile int d = 0; d < M * 2; ++d) {
            /* Mix of dummy calls similar to outer but in different order */
            if (d % 3 == 0) {
                dummy2(d);
                goto d_shared;
            } else {
                dummy3(d);
            }
            
        d_shared:
            dummy4(d + setup);
            
            /* Small conditional to create more blocks */
            if (d % 5 == 0) {
                volatile int temp = d * 2;
                dummy1(temp);
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", counter);
    
    /* Force use of all dummy functions */
    dummy1(1);
    dummy2(2);
    dummy3(3);
    dummy4(4);
    
    return 0;
}
