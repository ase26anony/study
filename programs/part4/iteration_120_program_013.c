/* Test program for hw-doloop.cc partial overlap bitmap analysis */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) void block_marker_a(int id) {
    asm volatile("" : : "r"(id) : "memory");
}

__attribute__((noinline, noclone)) void block_marker_b(int id) {
    asm volatile("" : : "r"(id) : "memory");
}

__attribute__((noinline, noclone)) void block_marker_c(int id) {
    asm volatile("" : : "r"(id) : "memory");
}

__attribute__((noinline, noclone)) void block_marker_d(int id) {
    asm volatile("" : : "r"(id) : "memory");
}

__attribute__((noinline, noclone)) void block_marker_e(int id) {
    asm volatile("" : : "r"(id) : "memory");
}

int main(void) {
    /* Volatile variables to prevent constant propagation and optimization */
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int L = 75;
    volatile int condition = 0;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int shared_counter = 0;
    block_marker_a(0);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        block_marker_b(outer);
        
        /* This if-else creates two distinct paths in the outer loop */
        if (condition) {
            /* TRUE BRANCH - contains Inner Loop A */
            block_marker_c(1);
            
            /* INNER LOOP A - starts inside true branch */
            volatile int inner_a;
            for (inner_a = 0; inner_a < M; ++inner_a) {
                block_marker_d(inner_a);
                checksum += inner_a * 2;
                
                /* This goto creates partial overlap:
                   Jumps to a block that's in outer loop but outside true branch */
                if (inner_a == M/2) {
                    goto shared_continuation;
                }
            }
            /* End of Inner Loop A normal path */
            block_marker_e(2);
            
shared_continuation:
            /* This block is in outer loop, executed by:
               1. Normal fall-through from else branch
               2. goto from Inner Loop A
               3. Normal fall-through from true branch (when goto not taken) */
            block_marker_a(3);
            checksum += outer * 3;
            
        } else {
            /* FALSE BRANCH - contains Inner Loop B */
            block_marker_c(4);
            
            /* INNER LOOP B - shares prologue but different body */
            volatile int inner_b;
            for (inner_b = 0; inner_b < K; ++inner_b) {
                block_marker_d(inner_b + 1000);  /* Different argument */
                checksum -= inner_b;
                asm volatile("" : : : "memory");  /* Barrier */
            }
            
            /* Fall through to shared continuation block */
            block_marker_e(5);
            goto shared_continuation;
        }
        
        /* Outer loop continuation after shared block */
        block_marker_b(outer + 1000);
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    block_marker_a(6);
    
    /* SIBLING LOOP C - shares prologue with inner loops but different body */
    volatile int sibling;
    for (sibling = 0; sibling < L; ++sibling) {
        /* Partially overlaps with outer loop blocks:
           - Uses block_marker_a (shared prologue)
           - Uses different markers in body */
        block_marker_c(sibling + 2000);
        checksum += sibling * sibling;
        
        /* Alternate between two different blocks to create complexity */
        if (sibling % 2) {
            block_marker_d(sibling + 3000);
        } else {
            block_marker_e(sibling + 4000);
        }
        
        asm volatile("" : : : "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int result = checksum % 1000;
    printf("Result: %d\n", result);
    
    return result;
}
