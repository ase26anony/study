/* Test program for hardware loop optimization analysis */
/* Designed to create nested loops with partial basic block overlap */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 7 + j * 13) % 3;
}

/* Function with side effects */
void record_result(int idx, int val) {
    results[idx] = val;
    checksum += val;
}

/* Test case 1: Three-level nested loops with partial overlap */
void test_case_1() {
    int counter = 0;
    
    /* Outer loop - will contain partial blocks of inner loops */
    for (int i = 0; i < 50; ++i) {
        /* Code executed in outer loop but not in inner loops */
        record_result(i, i * 2);
        
        /* Conditional that creates partial overlap */
        if (get_condition(i, 0) > 0) {
            /* Middle loop - partially overlapping with outer */
            for (int j = 0; j < 30; ++j) {
                /* Code in middle loop only */
                record_result(100 + j, i + j);
                
                /* Another conditional inside middle loop */
                if (get_condition(i, j) == 1) {
                    /* Inner loop - fully contained in middle loop's conditional branch */
                    for (int k = 0; k < 20; ++k) {
                        record_result(200 + k, i * j * k);
                    }
                } else {
                    /* Alternative path in middle loop - not containing inner loop */
                    record_result(300 + j, i - j);
                }
            }
        } else {
            /* Alternative path in outer loop - no inner loops here */
            record_result(400 + i, i * 3);
        }
    }
}

/* Test case 2: Sibling loops inside outer loop */
void test_case_2() {
    /* Outer loop containing two sibling inner loops */
    for (int i = 0; i < 40; ++i) {
        /* Pre-inner loop code */
        record_result(500 + i, i);
        
        /* First inner loop - executed conditionally */
        if (i % 3 == 0) {
            for (int j = 0; j < 25; ++j) {
                record_result(600 + j, i * 10 + j);
            }
        }
        
        /* Code between sibling loops */
        record_result(700 + i, i * 100);
        
        /* Second inner loop - different condition */
        if (i % 4 == 0) {
            for (int k = 0; k < 20; ++k) {
                record_result(800 + k, i * 20 + k);
            }
        }
        
        /* Post-inner loop code */
        record_result(900 + i, i * 1000);
    }
}

/* Test case 3: Complex partial overlap with early exits */
void test_case_3() {
    for (int a = 0; a < 35; ++a) {
        /* Multiple conditionals creating complex CFG */
        switch (a % 4) {
            case 0:
                /* Loop with early continue */
                for (int b = 0; b < 15; ++b) {
                    if (b % 2 == 0) continue;
                    record_result(1000 + b, a * b);
                    
                    /* Deeply nested under condition */
                    if (b > 5) {
                        for (int c = 0; c < 10; ++c) {
                            record_result(1100 + c, a + b + c);
                        }
                    }
                }
                break;
                
            case 1:
                /* Different loop structure */
                for (int b = 5; b < 20; ++b) {
                    record_result(1200 + b, a - b);
                }
                break;
                
            default:
                /* No inner loops in this path */
                record_result(1300 + a, a * a);
                break;
        }
    }
}

/* Test case 4: Loop with invariant code motion challenges */
void test_case_4() {
    volatile int mod = 7;  /* volatile to prevent hoisting */
    
    for (int x = 0; x < 45; ++x) {
        /* Loop-invariant calculation that can't be easily hoisted */
        int invariant = mod * x;
        
        /* Inner loop using the "invariant" value */
        for (int y = 0; y < 18; ++y) {
            /* Conditional execution creating partial overlap */
            if (y % 2 == 0) {
                record_result(1400 + y, invariant + y);
            } else {
                /* Different code path without inner loop */
                for (int z = 0; z < 8; ++z) {
                    record_result(1500 + z, invariant * y * z);
                }
            }
        }
        
        /* Additional outer loop code not in inner loop */
        mod = (mod + 1) % 11;
    }
}

int main() {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = 0;
    }
    
    /* Execute test cases to create various loop nesting patterns */
    printf("Starting hardware loop analysis tests...\n");
    
    test_case_1();  /* Three-level nesting with partial overlap */
    test_case_2();  /* Sibling loops in outer loop */
    test_case_3();  /* Complex partial overlap with switch */
    test_case_4();  /* Loop with invariant challenges */
    
    /* Final computation to ensure all loops executed */
    int final_sum = 0;
    for (int i = 0; i < 1000; ++i) {
        final_sum += results[i];
    }
    
    printf("Test completed. Final checksum: %d, Array sum: %d\n", 
           checksum, final_sum);
    
    /* Return non-zero if something went wrong (simplified check) */
    return (final_sum == 0) ? 1 : 0;
}
