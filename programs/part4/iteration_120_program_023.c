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
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common prologue block - shared by inner loops */
        volatile int shared = outer * 2;
        dummy1(shared);
        
        /* This creates two distinct paths in the outer loop */
        if (condition) {
            /* Inner Loop A - starts inside true branch */
            /* But will jump to shared block outside this branch */
            volatile int inner_a = 0;
            
            /* Label for goto - creates partial overlap */
            loop_a_start:
            for (inner_a = 0; inner_a < M; ++inner_a) {
                /* Unique body for loop A */
                dummy2(inner_a + shared);
                checksum += inner_a;
                
                /* Jump to block outside if branch but still in outer loop */
                /* This creates partial overlap: loop A blocks are both inside 
                   and outside the if branch of outer loop */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            
            /* Block only reached if no goto taken */
            dummy3(999);
        } else {
            /* Inner Loop B - different from loop A */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b - shared);
                checksum -= inner_b;
            }
        }
        
        /* Shared block - part of outer loop, reached by goto from loop A */
        /* This block is in outer loop's bitmap but not exclusively in 
           the if branch where loop A started */
        shared_block:
        dummy4(checksum);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both paths are taken */
        condition = !condition;
    }
    
    /* Sibling Loop C - shares prologue with inner loops but different body */
    /* This creates another partial overlap scenario */
    {
        volatile int shared = N * 3;  /* Similar prologue to inner loops */
        dummy1(shared);
        
        /* Loop C - sequential sibling */
        for (volatile int sibling = 0; sibling < N/2; ++sibling) {
            /* Different body from loops A and B */
            dummy2(sibling * 3);
            dummy3(sibling + shared);
            checksum += sibling * 2;
        }
    }
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use checksum to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
