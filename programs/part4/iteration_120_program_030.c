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
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory");
        
        /* This block is part of outer loop but will be shared */
    shared_block:
        dummy2(outer);
        
        if (condition) {
            /* INNER LOOP A - starts inside if branch */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                dummy3(inner_a);
                checksum += inner_a;
                
                /* Force partial overlap: jump to block outside if branch
                   but still within outer loop */
                if (inner_a == M/2) {
                    goto shared_block;  /* Creates partial overlap */
                }
            }
            /* End of if branch */
        } else {
            /* INNER LOOP B - alternative inner loop */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy4(inner_b);
                checksum -= inner_b;
            }
        }
        
        /* Continuation of outer loop after if-else */
        asm volatile("" : : : "memory");
        checksum += outer;
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    dummy1(shared_counter);
    
    /* SIBLING LOOP C - shares prologue but has different body */
    /* This creates bitmap intersection but not subset relationship */
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        asm volatile("" : : : "memory");
        dummy2(sibling);
        
        /* Different body from outer loop */
        for (volatile int inner_c = 0; inner_c < 10; ++inner_c) {
            dummy3(inner_c * 2);
            checksum += inner_c * 3;
        }
        
        checksum -= sibling;
    }
    
    /* Additional complexity: loop with switch to create more blocks */
    volatile int mode = 2;
    for (volatile int i = 0; i < 200; ++i) {
        switch (mode) {
            case 1:
                dummy1(i);
                break;
            case 2:
                dummy2(i);
                /* Fall through to create shared block */
            case 3:
                dummy3(i);
                break;
            default:
                dummy4(i);
        }
        checksum += i;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
