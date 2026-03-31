/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p checks in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERS 50

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_iteration(int loop_id, int i, int j, int k) {
    results[counter % SIZE] = loop_id * 1000 + i * 100 + j * 10 + k;
    counter++;
}

/* Test case 1: Inner loop fully contained within outer loop
 * This should trigger bitmap_intersect_p but not bitmap_intersect_compl_p
 * for the inner loop relative to outer loop
 */
void test_fully_contained(void) {
    for (int i = 0; i < ITERS; i++) {
        /* Code before inner loop - creates blocks in outer but not inner */
        if (i % 2 == 0) {
            results[i % SIZE] = i * 2;
        }
        
        /* Inner loop fully contained */
        for (int j = 0; j < ITERS / 2; j++) {
            record_iteration(1, i, j, 0);
            checksum += i * j;
        }
        
        /* Code after inner loop - more outer-only blocks */
        results[(i + 1) % SIZE] = i * 3 + 1;
    }
}

/* Test case 2: Partial overlap - inner loop shares some but not all blocks
 * This should trigger both bitmap_intersect_p and bitmap_intersect_compl_p
 */
void test_partial_overlap(void) {
    for (int i = 0; i < ITERS; i++) {
        /* Conditional that determines whether inner loop executes */
        int condition = get_condition(i, 0);
        
        if (condition == 0) {
            /* Branch with inner loop */
            for (int j = 0; j < ITERS / 3; j++) {
                record_iteration(2, i, j, 0);
                checksum += (i + j) * 2;
                
                /* Additional conditional inside inner loop */
                if (j % 4 == 0) {
                    results[(i + j) % SIZE] = j;
                }
            }
        } else if (condition == 1) {
            /* Alternative branch without inner loop */
            results[i % SIZE] = i * 5;
            checksum -= i;
        } else {
            /* Another branch with different inner loop structure */
            for (int j = 0; j < ITERS / 4; j++) {
                record_iteration(3, i, j, 0);
                checksum += i - j;
            }
            results[(i + 10) % SIZE] = i * 7;
        }
        
        /* Common code after conditional - in outer but not inner */
        checksum += i % 7;
    }
}

/* Test case 3: Three-level nesting with varying overlap patterns */
void test_three_level_nesting(void) {
    for (int i = 0; i < ITERS / 2; i++) {
        /* Outer loop code before middle loop */
        int outer_cond = i % 3;
        
        if (outer_cond == 0) {
            /* Middle loop A */
            for (int j = 0; j < ITERS / 3; j++) {
                /* Code before innermost loop */
                if (j % 2 == 0) {
                    results[(i + j) % SIZE] = i + j;
                }
                
                /* Innermost loop - fully contained in middle loop A */
                for (int k = 0; k < ITERS / 4; k++) {
                    record_iteration(4, i, j, k);
                    checksum += i * j * k;
                }
                
                /* Code after innermost loop */
                checksum -= j;
            }
        } else if (outer_cond == 1) {
            /* Middle loop B with different structure */
            for (int j = 0; j < ITERS / 5; j++) {
                /* Conditional inside middle loop B */
                if (get_condition(i, j) > 0) {
                    /* Partial innermost loop */
                    for (int k = 0; k < ITERS / 6; k++) {
                        record_iteration(5, i, j, k);
                        checksum += k;
                    }
                } else {
                    /* Alternative path without innermost loop */
                    results[(i * j) % SIZE] = i - j;
                }
            }
        } else {
            /* No middle loop in this branch */
            results[i % SIZE] = i * 11;
        }
        
        /* Outer loop code after conditional */
        checksum += i * 13;
    }
}

/* Test case 4: Sibling loops inside outer loop (not nested in each other)
 * Should create loops that are siblings in the hierarchy
 */
void test_sibling_loops(void) {
    for (int i = 0; i < ITERS; i++) {
        /* First inner loop (sibling A) */
        if (i % 4 != 0) {
            for (int j = 0; j < ITERS / 3; j++) {
                record_iteration(6, i, j, 0);
                checksum += i * 17 + j;
            }
        }
        
        /* Code between sibling loops */
        results[(i + 5) % SIZE] = i * 19;
        
        /* Second inner loop (sibling B) */
        if (i % 3 != 0) {
            for (int j = 0; j < ITERS / 4; j++) {
                record_iteration(7, i, j, 0);
                checksum += i * 23 - j;
                
                /* Additional conditional to create more blocks */
                if (j % 5 == 0) {
                    results[(i + j + 10) % SIZE] = j * 2;
                }
            }
        }
        
        /* More outer loop code */
        checksum += i % 11;
    }
}

/* Test case 5: Complex nested structure with early exits */
void test_complex_nesting(void) {
    for (int i = 0; i < ITERS; i++) {
        /* Multiple conditionals creating various paths */
        int path = get_condition(i, i);
        
        switch (path) {
            case 0:
                /* Path with deeply nested loops */
                for (int j = 0; j < ITERS / 2; j++) {
                    if (j > ITERS / 4) {
                        for (int k = 0; k < ITERS / 8; k++) {
                            record_iteration(8, i, j, k);
                            checksum += i + j + k;
                            
                            /* Early exit from innermost loop */
                            if (k == ITERS / 16) break;
                        }
                    } else {
                        results[(i + j) % SIZE] = j * 3;
                    }
                }
                break;
                
            case 1:
                /* Path with sequential inner loops */
                for (int j = 0; j < ITERS / 6; j++) {
                    record_iteration(9, i, j, 0);
                    checksum += i * j;
                }
                
                for (int j = 0; j < ITERS / 7; j++) {
                    record_iteration(10, i, j, 0);
                    checksum -= i * j;
                }
                break;
                
            default:
                /* Path with no inner loops */
                results[i % SIZE] = i * 29;
                break;
        }
        
        /* Final outer loop computation */
        checksum = checksum % 1000;
    }
}

int main(void) {
    /* Initialize random seed for get_condition function */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_fully_contained();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_partial_overlap();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_three_level_nesting();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 5 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum: %d\n", final_sum);
    printf("Total iterations recorded: %d\n", counter);
    
    return 0;
}
