/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
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

/* Function to create side effects that can't be optimized away */
int __attribute__((noinline)) side_effect(int x) {
    return x ^ 0x55AA;
}

/* Test case 1: Classic nested loops with full containment */
void test_full_containment(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Outer loop code not in inner loop */
        results[i % SIZE] = side_effect(i);
        
        for (int j = 0; j < MID_ITER; j++) {
            /* Middle loop code not in innermost loop */
            int temp = i * j;
            
            for (int k = 0; k < INNER_ITER; k++) {
                /* Innermost loop - fully contained in both outer loops */
                results[(i + j + k) % SIZE] += side_effect(temp + k);
                checksum ^= (i * j * k) & 0xFF;
            }
            
            /* More middle loop code after inner loop */
            counter += temp % 7;
        }
        
        /* More outer loop code after middle loop */
        if (i % 3 == 0) {
            results[i % SIZE] *= 2;
        }
    }
}

/* Test case 2: Partial overlap with conditional inner loop execution */
void test_partial_overlap_conditional(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Always executed in outer loop */
        int val = side_effect(i);
        
        if (val % 4 == 0) {
            /* Branch A: Contains an inner loop */
            for (int j = 0; j < MID_ITER; j++) {
                results[(i + j) % SIZE] = side_effect(val + j);
                checksum += j;
            }
        } else if (val % 4 == 1) {
            /* Branch B: Different code path, no inner loop */
            results[i % SIZE] = val * 3;
            counter++;
        } else {
            /* Branch C: Another inner loop with different structure */
            int acc = 0;
            for (int j = 5; j < MID_ITER + 5; j++) {
                acc += side_effect(j);
                /* Nested conditional inside inner loop */
                if (j % 2 == 0) {
                    for (int k = 0; k < INNER_ITER / 2; k++) {
                        results[(i + k) % SIZE] ^= acc + k;
                    }
                }
            }
            results[i % SIZE] = acc;
        }
        
        /* More outer loop code after conditional */
        checksum ^= val;
    }
}

/* Test case 3: Sibling loops with partial overlap */
void test_sibling_loops(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Common preamble for both siblings */
        int base = side_effect(i);
        
        /* First sibling loop */
        if (base % 3 != 0) {
            for (int j = 0; j < MID_ITER; j++) {
                results[(i * 2 + j) % SIZE] = base + j;
                checksum += 1;
            }
        }
        
        /* Code between sibling loops (not in either inner loop) */
        int intermediate = base * 2 + counter;
        
        /* Second sibling loop */
        if (base % 5 != 0) {
            for (int k = 0; k < INNER_ITER; k++) {
                results[(i * 3 + k) % SIZE] = intermediate - k;
                /* Additional conditional inside */
                if (k % 3 == 0) {
                    counter ^= k;
                }
            }
        }
        
        /* Common postamble */
        checksum ^= intermediate;
    }
}

/* Test case 4: Complex nesting with multiple exit points */
void test_complex_nesting(void) {
    for (int i = 0; i < OUTER_ITER; i++) {
        int outer_val = rand() % 100;
        
        for (int j = 0; j < MID_ITER; j++) {
            /* Early exit from middle loop */
            if (outer_val < 10) break;
            
            int middle_val = side_effect(outer_val + j);
            
            /* Conditional inner loop execution */
            if (middle_val % 2 == 0) {
                for (int k = 0; k < INNER_ITER; k++) {
                    results[(i + j + k) % SIZE] = middle_val * k;
                    
                    /* Early exit from inner loop */
                    if (k > INNER_ITER / 2 && (middle_val + k) % 7 == 0) {
                        break;
                    }
                    
                    checksum = (checksum + 1) & 0xFFFF;
                }
            } else {
                /* Alternative path without innermost loop */
                results[(i + j) % SIZE] = middle_val * 2;
            }
            
            /* More middle loop code */
            counter = (counter + j) & 0xFF;
        }
        
        /* Outer loop continuation */
        if (outer_val > 50) {
            results[i % SIZE] += outer_val;
        }
    }
}

/* Test case 5: Loop with invariant code motion opportunities */
void test_with_invariants(void) {
    int invariant = side_effect(42);  /* Loop invariant */
    
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Outer loop with invariant usage */
        int val = invariant * i;
        
        for (int j = 0; j < MID_ITER; j++) {
            /* Partially overlapping inner loop */
            if ((i + j) % 3 == 0) {
                for (int k = 0; k < INNER_ITER; k++) {
                    results[(val + j + k) % SIZE] = side_effect(k);
                }
            } else {
                /* Different basic blocks not in innermost loop */
                results[(val + j) % SIZE] = j * 2;
            }
            
            /* Middle loop code after conditional */
            checksum += val % 11;
        }
        
        /* More outer loop code */
        counter ^= val;
    }
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_full_containment();
    printf("Test 1 completed. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_partial_overlap_conditional();
    printf("Test 2 completed. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_sibling_loops();
    printf("Test 3 completed. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_complex_nesting();
    printf("Test 4 completed. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_with_invariants();
    printf("Test 5 completed. Checksum: %d, Counter: %d\n", checksum, counter);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum: %d\n", final_sum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
