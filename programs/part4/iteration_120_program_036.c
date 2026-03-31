/* Test program for hw-doloop.cc uncovered lines 429-436
 * Creates loops with partial bitmap overlap to trigger:
 * - bitmap_intersect_p() returns true
 * - bitmap_intersect_compl_p() returns true for both directions
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) void dummy1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy5(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main(void) {
    /* Use volatile to prevent constant propagation and optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 50;
    volatile int M3 = 75;
    volatile int cond = 1;
    
    volatile int checksum = 0;
    volatile int i, j, k, l;
    
    /* COMMON_PROLOGUE: A block that will be shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER_LOOP: Contains complex control flow with if-else */
    for (i = 0; i < N; ++i) {
        /* This block is part of outer loop only */
        dummy2(i);
        
        /* Branch that creates two different paths */
        if (cond) {
            /* INNER_LOOP_A: Starts inside the if branch */
            for (j = 0; j < M1; ++j) {
                /* Body of inner loop A */
                dummy3(j);
                checksum += j;
                
                /* CRITICAL: Jump to a block that's in outer loop 
                 * but outside the if branch */
                if (j == M1/2) {
                    goto shared_block;
                }
            }
            
            /* This block is only in the if branch */
            dummy4(123);
        } else {
            /* INNER_LOOP_B: Different inner loop in else branch */
            for (k = 0; k < M2; ++k) {
                /* Different body for loop B */
                dummy4(k);
                checksum -= k;
            }
        }
        
        /* Continue label for goto */
        shared_block:
        /* This block is in outer loop and shared by both branches
         * INNER_LOOP_A jumps here, making it partially overlap
         * with outer loop but not fully contained */
        dummy5(i);
        checksum += i * 2;
        
        /* Memory barrier to prevent loop fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    dummy1(shared_counter);
    
    /* SIBLING_LOOP_C: Shares the common prologue but has different body
     * This creates partial overlap with outer loop's structure */
    for (l = 0; l < M3; ++l) {
        /* Different body from inner loops */
        dummy2(l * 3);
        checksum += l * 3;
        
        /* Additional block to make bitmap different */
        if (l % 2) {
            dummy3(l);
        } else {
            dummy4(l);
        }
    }
    
    /* Final calculation to prevent dead code elimination */
    volatile int result = checksum;
    printf("Result: %d\n", result);
    
    return 0;
}
