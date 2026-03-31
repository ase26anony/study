/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

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

/* Shared prologue block - will be part of multiple loops */
int shared_prologue(volatile int *counter) {
    int val = *counter;
    asm volatile("" : "+r"(val) : : "memory");
    return val;
}

int main(void) {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int outer_cond = 1;
    volatile int checksum = 0;
    
    /* Common setup block - will be shared by multiple loops */
    volatile int common_val = 0;
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop only */
        dummy1(outer);
        
        /* Shared prologue - part of outer loop AND inner loops */
        common_val = shared_prologue(&outer);
        
        /* Complex if-else creates multiple basic blocks in outer loop */
        if (outer_cond) {
            /* Branch 1 - true path */
            dummy2(common_val);
            
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int inner_a;
            for (inner_a = 0; inner_a < M; ++inner_a) {
                /* Body of inner loop A */
                dummy3(inner_a + common_val);
                
                /* CRITICAL: goto to block outside if branch but still in outer loop */
                /* This creates partial overlap - inner loop has blocks both inside
                   and outside the if branch of outer loop */
                if (inner_a == M/2) {
                    goto shared_block;
                }
                
                /* More inner loop A body */
                checksum += inner_a;
            }
            
            /* End of if true branch */
            dummy4(common_val);
        } else {
            /* Branch 2 - false path */
            dummy2(common_val + 1);
            
            /* INNER LOOP B - different from A but shares prologue */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b + common_val + 1);
                checksum -= inner_b;
            }
        }
        
        /* Shared block - part of outer loop, jumped to from inner loop A */
        shared_block:
        asm volatile("" : : : "memory");
        
        /* More outer loop body */
        checksum += outer;
        
        /* Change condition to alternate branches */
        outer_cond = !outer_cond;
    }
    
    /* SIBLING LOOP C - shares common prologue but different body */
    /* This creates another partial overlap scenario */
    volatile int sibling_init = shared_prologue(&checksum);
    
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Partially overlaps with outer loop's prologue block */
        if (sibling % 2) {
            dummy1(sibling + sibling_init);
        } else {
            dummy2(sibling + sibling_init);
        }
        checksum += sibling * 2;
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
