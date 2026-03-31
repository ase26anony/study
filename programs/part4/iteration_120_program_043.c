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
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop only */
        dummy2(outer);
        
        /* Branch creates multiple basic blocks within outer loop */
        if (cond) {
            /* Inner Loop A - starts inside if branch */
            /* But will jump to shared_block which is outside if branch */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                /* Unique body for inner loop A */
                dummy3(inner_a);
                checksum += inner_a;
                
                /* Jump to shared block - creates partial overlap */
                /* This goto makes inner loop A extend beyond the if branch */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            /* End of inner loop A normal path */
            dummy4(123);
        } else {
            /* Inner Loop B - alternative inner loop */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                /* Unique body for inner loop B */
                dummy4(inner_b);
                checksum -= inner_b;
            }
        }
        
        /* Shared block - part of outer loop, outside if-else */
        /* Inner Loop A jumps here, creating partial overlap */
        shared_block:
        asm volatile("" : : : "memory");
        dummy1(checksum);
        
        /* Another basic block in outer loop */
        checksum += outer;
    }
    
    /* Memory barrier to prevent loop fusion */
    asm volatile("" : : : "memory");
    
    /* Sibling Loop C - shares common prologue but different body */
    /* Re-initialize shared_counter to reuse the prologue block */
    shared_counter = 0;
    dummy1(shared_counter);  /* Same as prologue above */
    
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Different body from other loops */
        dummy2(sibling * 2);
        checksum += sibling * 3;
        asm volatile("" : : : "memory");
    }
    
    /* Another sibling loop with partial overlap */
    volatile int temp = checksum;
    dummy1(temp);  /* Another use of dummy1, creating potential overlap */
    
    for (volatile int sibling2 = 0; sibling2 < M; ++sibling2) {
        /* Mix of operations from different loops */
        if (sibling2 % 2) {
            dummy3(sibling2);
        } else {
            dummy4(sibling2);
        }
        checksum += sibling2;
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
