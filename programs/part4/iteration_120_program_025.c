/* Test program for hw-doloop.cc uncovered lines 429-436 */
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

/* Shared prologue block - will be part of multiple loop bitmaps */
int __attribute__((noinline)) shared_prologue(int base) {
    volatile int shared = base * 2;
    asm volatile("" : : : "memory");
    return shared;
}

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int outer_cond = 1;
    volatile int checksum = 0;
    
    /* Common setup block - will be shared by multiple loops */
    int common_val = shared_prologue(42);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop only */
        dummy1(outer);
        
        /* Branch creates separate basic blocks within outer loop */
        if (outer_cond) {
            /* INNER LOOP A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            volatile int inner_a = 0;
            
            /* Label for goto - creates partial overlap */
            loop_a_start:
            for (; inner_a < M; ++inner_a) {
                /* Body of inner loop A */
                dummy2(inner_a + common_val);
                checksum += inner_a;
                
                /* Jump to shared block outside the if branch */
                /* This creates partial overlap: inner loop contains
                   blocks both inside and outside the if branch */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            
            /* Block only in inner loop A's true path */
            dummy3(inner_a);
        } else {
            /* INNER LOOP B - alternative inner loop */
            /* Shares common_val but has different body */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy4(inner_b - common_val);
                checksum -= inner_b;
            }
        }
        
        /* Shared block - part of outer loop, also jumped to from inner loop A */
        /* This creates the partial overlap condition */
        shared_block:
        asm volatile("" : : : "memory");
        checksum += outer;
        
        /* Reset condition for next iteration */
        outer_cond = !outer_cond;
    }
    
    /* SIBLING LOOP C - shares common_val but not all blocks with outer loop */
    /* This creates bitmap_intersect_compl_p conditions */
    volatile int sibling_val = common_val;
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Uses shared_prologue like outer loop, but different body */
        int local_common = shared_prologue(sibling);
        
        /* Different dummy function calls create distinct basic blocks */
        dummy1(sibling + local_common);
        dummy3(sibling - local_common);
        
        checksum += sibling * 2;
        
        /* Memory barrier prevents fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Another loop that shares setup but has different structure */
    /* Creates more complex bitmap relationships */
    volatile int temp = common_val;
    for (volatile int i = 0; i < K; ++i) {
        /* Jump back to loop_a_start creates cross-loop flow */
        /* This further complicates the bitmap relationships */
        if (i == K/3) {
            temp = shared_prologue(i);
            /* This goto creates a connection between loops */
            /* but not a proper nesting relationship */
            if (checksum > 1000) {
                goto loop_a_start;  /* Creates partial overlap */
            }
        }
        dummy2(i + temp);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0;
}
