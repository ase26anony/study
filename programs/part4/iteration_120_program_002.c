/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

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
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop only */
        dummy1(outer);
        
        /* Complex if-else structure creates multiple basic blocks */
        if (cond) {
            /* Branch 1 - will contain Inner Loop A */
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                /* Loop A body block */
                dummy2(inner_a);
                checksum += inner_a;
                
                /* CRITICAL: goto to block outside if branch but still in outer loop */
                if (inner_a == M/2) {
                    goto shared_block;  /* Creates partial overlap */
                }
            }
            
            /* This block is only executed if goto not taken */
            dummy3(999);
        } else {
            /* Branch 2 - contains Inner Loop B */
            
            /* INNER LOOP B - shares prologue but different body */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b);
                checksum -= inner_b;
            }
        }
        
        /* Label for goto target - this block is in outer loop but outside if branch */
        shared_block:
        dummy4(shared_counter++);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING LOOP C - shares some blocks with outer loop */
    /* First, reuse the shared_counter initialization */
    volatile int temp = shared_counter;
    
    /* Loop C uses same prologue pattern as inner loops but different body */
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* This block is similar to outer loop's prologue but not identical */
        dummy1(sibling);
        checksum += sibling * 2;
        
        /* Different body from outer loop */
        dummy4(sibling);
        
        asm volatile("" : : : "memory");
    }
    
    /* Another loop that partially overlaps with sibling loop */
    volatile int counter2 = temp;
    for (volatile int overlap = N/4; overlap < N/2; ++overlap) {
        /* Shares dummy1 call with sibling loop but different condition */
        if (overlap % 2) {
            dummy1(overlap);
        } else {
            dummy2(overlap);
        }
        checksum += overlap;
        counter2++;
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
