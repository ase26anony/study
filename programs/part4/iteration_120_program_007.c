/* Test program for hw-doloop.cc partial overlap bitmap analysis */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) 
void marker_a(int x) { 
    asm volatile("" : : "r"(x) : "memory"); 
}

__attribute__((noinline, noclone))
void marker_b(int x) { 
    asm volatile("" : : "r"(x) : "memory"); 
}

__attribute__((noinline, noclone))
void marker_c(int x) { 
    asm volatile("" : : "r"(x) : "memory"); 
}

__attribute__((noinline, noclone))
void marker_d(int x) { 
    asm volatile("" : : "r"(x) : "memory"); 
}

/* Shared block that will be part of multiple loops */
__attribute__((noinline, noclone))
int shared_prologue(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x * 2;
}

int main(void) {
    /* Volatile variables to prevent constant propagation */
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int counter = 0;
    
    /* COMMON BLOCK: This block will be shared by multiple loops */
    int shared_val = shared_prologue(42);
    
    /* OUTER LOOP: Contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This if-else creates two distinct paths in the outer loop */
        if (cond) {
            /* Branch 1: Contains Inner Loop A */
            
            /* PRE_A: Block that's ONLY in Inner Loop A's path */
            marker_a(1);
            
            /* INNER LOOP A: Starts inside if branch but will jump outside */
            volatile int j = 0;
            
            /* Label for goto - creates partial overlap */
            loop_a_start:
            
            for (; j < M; ++j) {
                /* Body of Inner Loop A */
                marker_b(j);
                counter += j;
                
                /* CRITICAL: Jump to a block that's in outer loop 
                   but outside this if branch */
                if (j == M/2) {
                    /* This goto creates partial overlap - Inner Loop A
                       will have blocks both inside and outside the if branch */
                    goto shared_outer_block;
                }
            }
            
            /* POST_A: Block only reached if no goto taken */
            marker_c(2);
            
            /* Jump over else branch */
            goto after_else;
        } else {
            /* Branch 2: Contains Inner Loop B */
            marker_d(3);
            
            /* INNER LOOP B: Uses the same shared_prologue but different body */
            for (volatile int k = 0; k < K; ++k) {
                marker_a(k + 100);
                counter -= k;
                asm volatile("" : : : "memory");
            }
        }
        
        /* SHARED_OUTER_BLOCK: This block is in outer loop and will be 
           reached by Inner Loop A via goto, creating partial overlap */
        shared_outer_block:
        marker_b(shared_val);
        counter += i;
        
        after_else:
        /* Continue outer loop */
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared value for sibling loop */
    shared_val = shared_prologue(24);
    
    /* SIBLING LOOP C: Shares some blocks with outer loop but not all */
    /* This creates the condition: bitmap_intersect_p && bitmap_intersect_compl_p */
    volatile int p = 0;
    
    /* Prologue that's similar to outer loop's prologue */
    int temp = shared_prologue(shared_val);
    
    for (; p < N/2; ++p) {
        /* Body partially overlaps with outer loop's structure */
        if (p % 3 == 0) {
            marker_c(p);
            counter += temp;
        } else {
            marker_d(p);
            counter -= temp;
        }
        
        /* But has unique blocks too */
        marker_a(p * 2);
        asm volatile("" : : : "memory");
    }
    
    /* Another sibling-like structure with different iteration count */
    volatile int q = 0;
    for (; q < N/3; ++q) {
        /* Mix of blocks from different loops */
        if (q % 2 == 0) {
            marker_b(q);
        } else {
            marker_c(q);
            /* Additional unique block */
            asm volatile("nop" : : : "memory");
        }
        counter += q;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", counter);
    
    return 0;
}
