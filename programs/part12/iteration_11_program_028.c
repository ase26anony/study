/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define NEST_LEVELS 3

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int outer_counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Function with side effects that can't be optimized away */
void record_result(int idx, int val) {
    results[idx % SIZE] = val;
    checksum ^= val;
}

int main(void) {
    int i, j, k;
    
    /* Initialize random seed for unpredictable but deterministic behavior */
    srand(42);
    
    /* Test Case 1: Classic nested loops with full containment */
    printf("Test 1: Fully nested loops\n");
    for (i = 0; i < 50; i++) {
        outer_counter++;
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 20; k++) {
                record_result(i + j + k, i * j * k);
            }
        }
    }
    
    /* Test Case 2: Partial overlap - inner loop in conditional branch */
    printf("Test 2: Partial overlap with conditional inner loop\n");
    for (i = 0; i < 100; i++) {
        /* This block is in outer loop but NOT in inner loop */
        int temp = rand() % 100;
        record_result(i, temp);
        
        if (get_condition(i, 0) > 2) {
            /* Inner loop - shares some blocks with outer but not all */
            for (j = 0; j < 40; j++) {
                /* Additional condition inside inner loop */
                if (j % 3 == 0) {
                    record_result(i + j, i * j + temp);
                } else {
                    record_result(i - j, i * j - temp);
                }
            }
        } else {
            /* Alternative path - creates blocks in outer but not in inner */
            for (j = 0; j < 25; j++) {
                record_result(j, i * 100 + j);
            }
        }
    }
    
    /* Test Case 3: Sibling loops with partial overlap */
    printf("Test 3: Sibling loops in outer loop\n");
    for (i = 0; i < 80; i++) {
        /* First inner loop */
        if (i % 4 == 0) {
            for (j = 0; j < 35; j++) {
                record_result(i * 10 + j, (i + j) * 3);
            }
        }
        
        /* Some code between sibling loops */
        int mid = i * 7 % 19;
        record_result(mid, mid * 2);
        
        /* Second inner loop (sibling of first) */
        if (i % 3 == 0) {
            for (j = 10; j < 45; j++) {
                record_result(i * 5 + j, (i - j) * 4);
            }
        }
    }
    
    /* Test Case 4: Complex nesting with multiple exit points */
    printf("Test 4: Complex nesting with early exits\n");
    for (i = 0; i < 60; i++) {
        for (j = 0; j < 50; j++) {
            /* Conditional that may skip the innermost loop */
            if (get_condition(i, j) != 0) {
                for (k = 0; k < 30; k++) {
                    if (k % 5 == 0) {
                        record_result(i * j * k, i + j + k);
                        /* Early continue creates additional basic blocks */
                        if (k == 15) continue;
                    }
                }
            }
            
            /* Code that executes regardless of innermost loop */
            record_result(i * 100 + j, j * 200);
        }
        
        /* Early break from outer loop */
        if (i == 45) {
            record_result(999, i);
            break;
        }
    }
    
    /* Test Case 5: Nested loops with invariant code motion challenges */
    printf("Test 5: Loops with hard-to-hoist operations\n");
    volatile int barrier = 0;
    for (i = 0; i < 70; i++) {
        barrier = i;  /* Volatile write prevents hoisting */
        
        for (j = 0; j < 40; j++) {
            /* Function call with side effects */
            int r = rand() % 50;
            
            if (r > 25) {
                for (k = 0; k < 25; k++) {
                    /* Complex addressing with side effects */
                    results[(i + j + k) % SIZE] = i * j * k + r;
                    checksum += results[(i + j + k) % SIZE];
                }
            } else {
                /* Different path with its own inner loop */
                for (k = 10; k < 30; k++) {
                    results[(i * j + k) % SIZE] = i - j + k;
                    checksum -= results[(i * j + k) % SIZE];
                }
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("Outer counter: %d\n", outer_counter);
    printf("Sample results[0..4]: %d %d %d %d %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    
    return 0;
}
