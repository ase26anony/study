/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p checks in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 3 + j * 7) % 5;
}

/* Function with side effects that can't be optimized away */
void record_result(int value) {
    results[counter % SIZE] = value;
    counter++;
}

/* Test case 1: Outer loop with inner loop in one branch only */
void test_partial_overlap_1(void) {
    volatile int x = 0;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        /* This block is in outer loop but NOT in inner loop */
        x += i * 2;
        record_result(x);
        
        /* Conditional that creates partial overlap */
        if (get_condition(i, 0) > 2) {
            /* Inner loop - its blocks are subset of outer's blocks */
            for (int j = 0; j < MID_ITER; j++) {
                x += j * 3;
                /* Another level of nesting with different overlap pattern */
                if (j % 3 == 0) {
                    for (int k = 0; k < INNER_ITER; k++) {
                        x += k * 5;
                        record_result(x);
                    }
                } else {
                    /* Alternative path in middle loop */
                    x += 100;
                }
            }
        } else {
            /* Alternative path in outer loop - NOT in inner loop */
            x -= 50;
            record_result(x * 2);
        }
        
        /* More outer loop code not in any inner loop */
        x += i % 10;
    }
    
    checksum += x;
}

/* Test case 2: Two sibling inner loops with partial overlap */
void test_sibling_loops(void) {
    volatile int y = 1000;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Common outer loop code */
        y += i;
        
        /* First inner loop under condition */
        if (i % 4 == 0) {
            for (int j = 0; j < MID_ITER; j++) {
                y += j * 2;
                record_result(y);
                
                /* Deeply nested with full containment */
                for (int k = 0; k < INNER_ITER; k++) {
                    y += k;
                }
            }
        }
        
        /* Code between sibling loops - in outer but not in first inner */
        y -= i / 2;
        
        /* Second inner loop under different condition */
        if (i % 3 == 0) {
            for (int j = 0; j < MID_ITER; j++) {
                y += j * 3;
                /* Different control flow inside */
                if (j % 2 == 0) {
                    for (int k = 0; k < INNER_ITER; k++) {
                        y += k * 2;
                    }
                }
            }
        }
        
        /* More outer-only code */
        y += i % 7;
    }
    
    checksum += y;
}

/* Test case 3: Complex nesting with mixed containment */
void test_complex_nesting(void) {
    volatile int z = 2000;
    
    for (int a = 0; a < 40; a++) {
        z += a;
        
        /* Level 2 loop - partially overlapping with outer */
        for (int b = 0; b < 25; b++) {
            /* Code in level 2 but not in level 3 */
            z += b * 2;
            
            if (b % 3 == 0) {
                /* Level 3 loop - fully contained in level 2 */
                for (int c = 0; c < 15; c++) {
                    z += c;
                    record_result(z);
                    
                    /* Level 4 - partially overlapping with level 3 */
                    if (c % 4 == 0) {
                        for (int d = 0; d < 10; d++) {
                            z += d * 3;
                        }
                    } else {
                        /* Alternative path in level 3 */
                        z -= 5;
                    }
                }
            } else {
                /* Alternative path in level 2 */
                z += 20;
            }
        }
        
        /* More outer-only code creating partial overlap */
        if (a % 5 == 0) {
            z *= 2;
        }
    }
    
    checksum += z;
}

/* Test case 4: Loop with early exit creating complex CFG */
void test_early_exit(void) {
    volatile int w = 3000;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        w += i;
        
        /* Inner loop with break */
        for (int j = 0; j < MID_ITER; j++) {
            w += j;
            
            if (w > 4000) {
                /* Early exit creates separate basic block */
                break;
            }
            
            /* Nested loop inside */
            for (int k = 0; k < INNER_ITER; k++) {
                w += k % 5;
                record_result(w);
            }
        }
        
        /* Conditional that might skip some iterations */
        if (i % 6 == 0) {
            continue;
        }
        
        w -= 10;
    }
    
    checksum += w;
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_1();
    printf("Test 1 completed, checksum so far: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 completed, checksum so far: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 3 completed, checksum so far: %d\n", checksum);
    
    test_early_exit();
    printf("Test 4 completed, checksum so far: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results array sum: %d\n", final_sum);
    printf("Total checksum: %d\n", checksum);
    printf("Counter: %d\n", counter);
    
    /* Return non-zero if something went wrong (all loops optimized out) */
    if (counter == 0 || checksum == 0) {
        printf("ERROR: Loops may have been optimized away!\n");
        return 1;
    }
    
    return 0;
}
