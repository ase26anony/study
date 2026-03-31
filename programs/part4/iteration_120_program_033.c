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
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        checksum += outer;
        
        /* Complex if-else structure creating multiple basic blocks */
        if (condition) {
            /* Branch 1 - will contain Inner Loop A */
            dummy2(1);
            
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int inner_a = 0;
        inner_loop_a_start:
            for (; inner_a < M; ++inner_a) {
                dummy3(inner_a);
                checksum += inner_a * 2;
                
                /* Critical: Jump to a block outside the if branch
                   but still within outer loop */
                if (inner_a == M/2) {
                    goto shared_block;  /* Creates partial overlap */
                }
            }
            
            /* Block only reached if no goto taken */
            dummy4(1000);
            continue;  /* Skip the else branch */
            
        shared_block:
            /* This block is part of outer loop but outside the if branch */
            dummy4(2000);
            /* Continue to code after if-else */
        } else {
            /* Branch 2 - contains Inner Loop B */
            dummy2(2);
            
            /* INNER LOOP B - shares prologue but different body */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b + 100);
                checksum -= inner_b;
            }
            
            /* Different termination block */
            dummy4(3000);
        }
        
        /* Common epilogue for outer loop (after if-else) */
        volatile int epilogue_var = outer * 3;
        dummy1(epilogue_var);
    }
    
    /* Memory barrier between loops */
    asm volatile("" : : : "memory");
    
    /* SIBLING LOOP C - shares the common prologue block 
       but has partial overlap with outer loop */
    /* Reset shared counter to reuse prologue */
    shared_counter = 0;
    dummy1(shared_counter);
    
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Different body from other loops */
        dummy2(sibling + 1000);
        checksum += sibling * 3;
        
        /* Different control flow pattern */
        if (sibling % 3 == 0) {
            dummy3(sibling);
        } else {
            dummy4(sibling);
        }
    }
    
    /* Final computation to prevent elimination */
    volatile int result = checksum;
    printf("Result: %d\n", result);
    
    return result != 0;
}
