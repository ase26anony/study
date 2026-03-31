/* Test program for hardware loop nested loop analysis */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Global arrays to prevent optimization */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
void side_effect(int value) {
    results[counter % SIZE] = value;
    counter++;
}

/* Test case 1: Inner loop fully contained within outer loop */
void test_fully_contained(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Code before inner loop - creates basic blocks not in inner loop */
        if (i % 3 == 0) {
            side_effect(i * 2);
        }
        
        /* Inner loop - fully contained */
        for (int j = 0; j < MID_ITER; j++) {
            results[(i * j) % SIZE] = i + j;
            if (j % 5 == 0) {
                side_effect(j);
            }
        }
        
        /* Code after inner loop - more blocks not in inner loop */
        if (i % 4 == 0) {
            side_effect(i * 3);
        }
    }
}

/* Test case 2: Partially overlapping loops with conditional execution */
void test_partial_overlap(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* This block is in outer but not in inner when condition is false */
        if (i % 2 == 0) {
            /* Inner loop - only executes sometimes */
            for (int j = 0; j < MID_ITER; j++) {
                results[(i + j) % SIZE] = i * j;
                /* Nested inner-inner loop */
                for (int k = 0; k < INNER_ITER; k++) {
                    checksum += k;
                    if (k % 3 == 0) {
                        side_effect(k);
                    }
                }
            }
        } else {
            /* Alternative path - creates blocks in outer but not inner */
            side_effect(i * 100);
            for (int j = 0; j < 10; j++) {
                checksum -= j;
            }
        }
        
        /* Common code after the if-else */
        results[i % SIZE] = i;
    }
}

/* Test case 3: Sibling loops inside an outer loop */
void test_sibling_loops(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* First inner loop */
        if (i % 3 != 0) {
            for (int j = 0; j < MID_ITER; j++) {
                results[(i * 2 + j) % SIZE] = rand() % 100;
                checksum += j;
            }
        }
        
        /* Code between sibling loops */
        side_effect(i);
        
        /* Second inner loop (sibling of first) */
        if (i % 4 != 0) {
            for (int k = 0; k < INNER_ITER; k++) {
                results[(i * 3 + k) % SIZE] = k * 2;
                checksum -= k;
                /* Very inner loop */
                for (int m = 0; m < 5; m++) {
                    side_effect(m * i);
                }
            }
        }
        
        /* More outer-only code */
        if (i % 5 == 0) {
            checksum += i * 2;
        }
    }
}

/* Test case 4: Complex nesting with multiple exit points */
void test_complex_nesting(void) {
    int early_exit = 0;
    
    for (int i = 0; i < OUTER_ITER && !early_exit; i++) {
        /* Outer loop has multiple basic blocks */
        switch (i % 4) {
            case 0:
                for (int j = 0; j < MID_ITER; j++) {
                    results[j % SIZE] = i + j;
                    if (j % 7 == 0) {
                        /* Early continue in middle loop */
                        continue;
                    }
                    for (int k = 0; k < INNER_ITER; k++) {
                        checksum += k * j;
                        if (checksum > 10000) {
                            early_exit = 1;
                            break;
                        }
                    }
                    if (early_exit) break;
                }
                break;
                
            case 1:
                /* Different inner loop structure */
                for (int j = 5; j < MID_ITER; j += 2) {
                    side_effect(j);
                }
                break;
                
            default:
                /* Outer-only code path */
                checksum += i * 3;
                break;
        }
        
        /* More outer loop code */
        results[i % SIZE] = checksum % 1000;
    }
}

/* Test case 5: Loop with invariant code that can't be hoisted */
void test_with_volatile_access(void) {
    volatile int seed = 42;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Volatile read creates side effect in outer loop */
        int val = seed;
        
        if (val % 3 == i % 3) {
            for (int j = 0; j < MID_ITER; j++) {
                /* Inner loop with its own volatile access */
                volatile int inner_val = j;
                results[(i + j) % SIZE] = inner_val + val;
                
                for (int k = 0; k < INNER_ITER; k++) {
                    checksum += k * inner_val;
                    if (k % 2 == 0) {
                        side_effect(k);
                    }
                }
            }
        } else {
            /* Alternative path without inner loop */
            checksum += val * i;
            side_effect(val);
        }
        
        /* Modify volatile variable */
        seed += i;
    }
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_fully_contained();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_partial_overlap();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    test_with_volatile_access();
    printf("Test 5 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum: %d\n", final_sum);
    printf("Total side effects: %d\n", counter);
    
    return 0;
}
