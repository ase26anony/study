/* test_hw_doloop.c
 * Designed to trigger uncovered bitmap intersection logic in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -fdump-rtl-doloop -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
#define HOT __attribute__((hot, noinline))
#else
#define HOT
#endif

/* Volatile variables to prevent constant propagation */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays for side effects */
int array1[200];
int array2[200];
int array3[200];

/* Function containing all loop patterns */
HOT void test_loop_patterns(void) {
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed for hardware loop */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop - should not intersect with Loop 1 */
    for (j = 0; j < M; j++) {
        array2[j] = j * 3;
        sum2 += array2[j];
    }
    
    /* Loop 3: Perfectly nested loop inside conditional */
    /* This creates hierarchical relationship */
    if (sum1 > 0) {
        /* Loop 3a: Inner loop entirely within if block */
        for (k = 0; k < K; k++) {
            array3[k] = k * 4;
            sum3 += array3[k];
        }
    }
    
    /* Loop 4: do-while loop for CFG variation */
    i = 0;
    do {
        array1[i] += 1;
        i++;
    } while (i < 10);
    
    /* Loop 5 and 6: Partially overlapping loops using goto */
    /* This creates bitmap intersection without complete subset relationship */
    
    /* Loop 5 */
    for (i = 0; i < N; i++) {
        array1[i] *= 2;
        
        /* Conditional that can jump into Loop 6 */
        if (i == N/2) {
            /* Label inside Loop 6's body */
            goto partial_overlap;
        }
        
        array2[i % M] += array1[i];
        
        /* Continue with Loop 5 */
        if (i < N-1) {
            continue;
        }
        
    partial_overlap:
        /* Loop 6: Starts here but shares some blocks with Loop 5 */
        for (j = 0; j < M; j++) {
            array2[j] -= 1;
            
            /* This block is shared between Loop 5 and Loop 6 */
            if (j == M/2) {
                /* Jump back to Loop 5's continuation */
                goto continue_loop5;
            }
            
            array3[j % K] += array2[j];
        }
        
    continue_loop5:
        /* Back to Loop 5's body */
        array1[i] /= 2;
    }
    
    /* Loop 7: Another loop with complex control flow for partial overlap */
    for (i = 0; i < N; i++) {
        if (i % 3 == 0) {
            /* This block will be shared with Loop 8 */
            for (j = 0; j < 5; j++) {
                array1[i] += j;
            }
            
            /* Loop 8: Nested but not perfectly - creates partial overlap */
            if (array1[i] > 100) {
                for (k = 0; k < 3; k++) {
                    array2[k] += array1[i];
                    /* Shared block with Loop 7's outer body */
                    if (k == 1) {
                        goto shared_block;
                    }
                }
            }
        }
        
    shared_block:
        /* This block belongs to both Loop 7 and Loop 8's CFG */
        array3[i % K] = i;
        
        /* Complex continue logic */
        if (i % 2 == 0) {
            continue;
        }
        
        /* More operations */
        array1[i] &= 0xFF;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Additional function with switch-case containing loops */
HOT void nested_switch_loops(void) {
    volatile int choice = 3;
    int x = 0;
    
    switch (choice) {
        case 1: {
            /* Loop in case 1 */
            for (int i = 0; i < 20; i++) {
                array1[i] += i;
            }
            break;
        }
        case 2: {
            /* Loop in case 2 - disjoint from case 1 */
            for (int i = 0; i < 15; i++) {
                array2[i] -= i;
            }
            break;
        }
        case 3: {
            /* Loop in case 3 with nested loop */
            for (int i = 0; i < 25; i++) {
                x += i;
                /* Inner loop creates hierarchy */
                for (int j = 0; j < 10; j++) {
                    array3[j] = x + j;
                }
            }
            break;
        }
    }
    
    /* Loop after switch - adjacent to case loops */
    for (int i = 0; i < 30; i++) {
        array1[i] ^= array2[i % 15];
    }
}

/* Main function to ensure execution */
int main(void) {
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < 200; i++) {
        array1[i] = i;
        array2[i] = 200 - i;
        array3[i] = 0;
    }
    
    /* Call functions with loop patterns */
    test_loop_patterns();
    nested_switch_loops();
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
