/* test_hw_doloop.c
 * Designed to trigger uncovered lines in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int M, int *result1, int *result2, int *result3) 
{
    volatile int limit = N;  /* Prevent constant propagation */
    volatile int inner_limit = M;
    int i, j, k;
    
    /* Array for side effects - prevents dead code elimination */
    volatile int side_effect[1024] = {0};
    
    /* ============================================
     * Loop 1: Simple countable loop (disjoint from others)
     * ============================================ */
    for (i = 0; i < limit; i++) {
        /* Simple body with side effect */
        side_effect[i % 1024] += i;
        *result1 += i * 2;
    }
    
    /* ============================================
     * Loop 2: Perfectly nested inside Loop 3
     * This should trigger: loop->loops.safe_push(other)
     * ============================================ */
    for (j = 0; j < limit; j++) {
        /* Outer loop body */
        side_effect[(j + 1) % 1024] ^= j;
        
        /* ============================================
         * Loop 3: Inner loop (perfectly nested)
         * Loop 2's blocks are a superset of Loop 3's blocks
         * ============================================ */
        for (k = 0; k < inner_limit; k++) {
            side_effect[(j + k) % 1024] += k;
            *result2 += j * k;
        }
        
        /* More outer loop body */
        side_effect[j % 1024] -= 1;
    }
    
    /* ============================================
     * Loop 4: Partially overlapping with Loop 5
     * This should trigger: other->loops.safe_push(loop)
     * ============================================ */
    int counter4 = 0;
    int counter5 = 0;
    
    /* Shared label for partial overlap */
    shared_block:
        side_effect[100] = counter4 + counter5;
    
    /* Loop 4 with conditional goto into Loop 5's region */
    for (counter4 = 0; counter4 < limit; counter4++) {
        *result3 += counter4 * 3;
        
        /* Conditional that creates partial overlap */
        if (counter4 % 3 == 0) {
            /* Jump to shared block that's also in Loop 5 */
            goto shared_block;
        }
        
        side_effect[counter4 % 1024] |= 0xFF;
        
        /* Return from shared block */
        continue_loop4:
            side_effect[200] = counter4;
    }
    
    /* ============================================
     * Loop 5: Partially overlapping with Loop 4
     * Shares the "shared_block" but has different entry/exit
     * ============================================ */
    for (counter5 = 0; counter5 < inner_limit; counter5++) {
        /* Entry to Loop 5 */
        side_effect[300] = counter5;
        
        /* Jump to the shared block */
        goto shared_block;
        
        /* This is part of Loop 5 but not Loop 4 */
        unique_to_loop5:
            *result3 -= counter5;
            side_effect[400] = counter5;
            
        /* Jump back to avoid infinite loop */
        goto end_loop5;
    }
    
    /* ============================================
     * Loop 6: Do-while loop for CFG variation
     * Disjoint from all others
     * ============================================ */
    int counter6 = 0;
    do {
        side_effect[500] = counter6;
        *result1 -= counter6;
        counter6++;
    } while (counter6 < limit);
    
    /* ============================================
     * Loop 7: While loop with complex condition
     * Adjacent to Loop 6 but disjoint
     * ============================================ */
    int counter7 = limit;
    while (counter7 > 0) {
        side_effect[600] = counter7;
        *result2 += counter7;
        counter7--;
    }
    
    /* Control flow labels for partial overlap */
    end_loop5:
        side_effect[700] = 1;
    goto continue_loop4;  /* Creates the partial overlap */
    
    /* Unreachable but needed for CFG */
    goto unique_to_loop5;
}

/* Main function to ensure execution */
int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int N = 100;
    volatile int M = 50;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, M, &result1, &result2, &result3);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = result1 + result2 + result3;
    
    /* Print to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    
    /* Additional volatile operations to prevent dead code */
    volatile int dummy = checksum;
    
    return checksum != 0 ? 0 : 1;
}
