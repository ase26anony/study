/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERATIONS 10

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[SIZE][SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_iteration(int i, int j, int k, int val) {
    results[i % SIZE][j % SIZE] = val;
    checksum = (checksum * 31 + val) & 0xFFFF;
    counter++;
}

/* Test case 1: Three-level nested loops with partial overlap in middle level */
void test_case_1(void) {
    printf("Test case 1: Three-level nesting with conditional inner loop\n");
    
    for (int i = 0; i < ITERATIONS; i++) {
        /* Outer loop block A */
        int outer_temp = i * 2;
        
        /* Conditional that creates partial overlap */
        if (i % 2 == 0) {
            /* This inner loop shares blocks with outer, but outer has additional blocks */
            for (int j = 0; j < ITERATIONS / 2; j++) {
                /* Middle loop block B (shared with outer when condition true) */
                int middle_temp = j + outer_temp;
                
                /* Innermost loop - fully contained */
                for (int k = 0; k < 3; k++) {
                    record_iteration(i, j, k, middle_temp + k);
                }
                
                /* Additional middle loop block not in innermost */
                if (j % 3 == 0) {
                    record_iteration(i, j, -1, middle_temp * 7);
                }
            }
            
            /* Additional outer block when condition true (not in middle loop) */
            record_iteration(i, -1, -1, outer_temp * 11);
        } else {
            /* Alternative path - outer loop blocks not in middle loop */
            for (int j = ITERATIONS / 2; j < ITERATIONS; j++) {
                record_iteration(i, j, -2, outer_temp + j * 5);
            }
        }
        
        /* Outer loop block C (always executed, not in middle loop) */
        record_iteration(i, -3, -3, outer_temp % 7);
    }
}

/* Test case 2: Sibling inner loops with different conditionals */
void test_case_2(void) {
    printf("Test case 2: Sibling loops with partial overlap\n");
    
    for (int i = 0; i < ITERATIONS; i++) {
        int base = i * 3;
        
        /* First inner loop under one condition */
        if (get_condition(i, 0) == 0) {
            for (int j = 0; j < 5; j++) {
                /* Blocks: outer + this inner loop */
                record_iteration(i, j, 100, base + j);
                
                /* Conditional inside inner loop creates more blocks */
                if (j % 2 == 0) {
                    record_iteration(i, j, 101, base * j);
                }
            }
        }
        
        /* Second inner loop under different condition - partial overlap with first */
        if (get_condition(i, 1) == 1) {
            for (int j = 2; j < 7; j++) {
                /* Different block pattern than first inner loop */
                record_iteration(i, j, 200, base - j);
                
                /* Nested conditional with different structure */
                if (j % 3 == 0) {
                    for (int k = 0; k < 2; k++) {
                        record_iteration(i, j, k + 300, base + j + k);
                    }
                } else {
                    record_iteration(i, j, 400, base * j / 2);
                }
            }
        }
        
        /* Outer loop code not in either inner loop */
        record_iteration(i, -4, -4, base % 11);
    }
}

/* Test case 3: Complex nested structure with multiple exit points */
void test_case_3(void) {
    printf("Test case 3: Complex nesting with early exits\n");
    
    for (int i = 0; i < ITERATIONS * 2; i++) {
        int val = i * 5;
        
        /* Loop with early exit possibility */
        for (int j = 0; j < ITERATIONS; j++) {
            if (val + j > 50) {
                /* Early exit creates separate basic block */
                record_iteration(i, j, 500, val);
                break;
            }
            
            /* Normal path */
            for (int k = 0; k < 4; k++) {
                /* Fully contained innermost loop */
                record_iteration(i, j, k, val + j + k);
                
                /* Conditional inside innermost */
                if (k == 2) {
                    record_iteration(i, j, k + 1000, val * k);
                }
            }
            
            /* Middle loop block not in innermost */
            if (j % 4 == 0) {
                record_iteration(i, j, 600, val - j);
            }
        }
        
        /* Additional outer loop block with its own mini-loop */
        for (int m = 0; m < 2; m++) {
            record_iteration(i, m + 100, 700, val + m);
        }
    }
}

/* Test case 4: Diamond-shaped control flow with nested loops */
void test_case_4(void) {
    printf("Test case 4: Diamond pattern with loops in branches\n");
    
    for (int i = 0; i < ITERATIONS; i++) {
        int cond = i % 4;
        
        switch (cond) {
            case 0:
                /* Branch A with simple inner loop */
                for (int j = 0; j < 3; j++) {
                    record_iteration(i, j, 800, i * j);
                }
                break;
                
            case 1:
                /* Branch B with deeper nesting */
                for (int j = 0; j < 4; j++) {
                    for (int k = 0; k < 2; k++) {
                        record_iteration(i, j, k + 900, i + j + k);
                    }
                }
                break;
                
            case 2:
                /* Branch C with conditional inner loop */
                if (i > ITERATIONS / 2) {
                    for (int j = 0; j < 5; j++) {
                        record_iteration(i, j, 1000, i - j);
                    }
                } else {
                    record_iteration(i, -5, -5, i * 3);
                }
                break;
                
            default:
                /* Branch D - no inner loop, just outer blocks */
                record_iteration(i, -6, -6, i * 7);
                break;
        }
        
        /* Common outer loop tail */
        record_iteration(i, -7, -7, i % 13);
    }
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    printf("Starting hardware loop analysis test...\n");
    printf("Target: bitmap_intersect_compl_p logic in hw-doloop.cc\n\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    
    /* Final computation to ensure all loops have observable effects */
    int final_sum = 0;
    for (int i = 0; i < SIZE && i < counter / SIZE + 1; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum += results[i][j];
        }
    }
    
    printf("\nTest completed.\n");
    printf("Total iterations: %d\n", counter);
    printf("Checksum: %d\n", checksum);
    printf("Final array sum: %d\n", final_sum);
    
    /* Return value based on test execution */
    return (counter > 0 && checksum != 0) ? 0 : 1;
}
