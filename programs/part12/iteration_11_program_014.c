/* Test program for hardware loop optimization analysis */
/* Designed to trigger bitmap_intersect_compl_p checks in hw-doloop.cc */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 7 + j * 13) % 5;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

/* Test case 1: Nested loops with conditional inner loop execution */
void test_partial_overlap_1(int n) {
    volatile int local_sum = 0;
    
    /* Outer loop - will contain some blocks not in inner loop */
    for (int i = 0; i < n; i++) {
        /* Code executed in outer loop but not in inner loop */
        local_sum += i * 2;
        record_value(counter++, i);
        
        /* Conditional execution of inner loop */
        if (get_condition(i, 0) > 1) {
            /* Inner loop - partially overlaps with outer loop */
            for (int j = 0; j < i + 5; j++) {
                local_sum += j * 3;
                record_value(counter++, j);
                
                /* Additional conditional inside inner loop */
                if (j % 3 == 0) {
                    local_sum += 7;
                }
            }
        } else {
            /* Alternative path in outer loop (not in inner loop) */
            local_sum += 11;
            record_value(counter++, 999);
        }
        
        /* More outer loop code not in inner loop */
        local_sum -= i;
    }
    
    results[0] = local_sum;
}

/* Test case 2: Three-level nesting with varying overlap patterns */
void test_three_level_nesting(int n) {
    volatile int acc = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; i++) {
        acc += i;
        
        /* Level 2: Middle loop - sometimes executed */
        if (i % 3 != 0) {
            for (int j = 0; j < (i % 5) + 2; j++) {
                acc += j * 2;
                
                /* Level 3: Innermost loop - conditionally executed */
                if ((i + j) % 2 == 0) {
                    for (int k = 0; k < (j % 3) + 1; k++) {
                        acc += k * 3;
                        record_value(counter++, k);
                    }
                } else {
                    /* Alternative path in middle loop */
                    acc += 5;
                }
            }
        }
        
        /* Additional outer loop code */
        acc -= i / 2;
    }
    
    results[1] = acc;
}

/* Test case 3: Sibling loops inside outer loop */
void test_sibling_loops(int n) {
    volatile int total = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (int i = 0; i < n; i++) {
        total += i * i;
        
        /* First inner loop - executed based on condition */
        if (i % 4 == 0) {
            for (int j = 0; j < (i % 7) + 3; j++) {
                total += j;
                record_value(counter++, j * 2);
            }
        }
        
        /* Code between sibling loops (in outer loop only) */
        total += 17;
        
        /* Second inner loop - different condition */
        if (i % 3 == 1) {
            for (int k = 0; k < (i % 6) + 2; k++) {
                total += k * k;
                record_value(counter++, k * 3);
                
                /* Nested conditional inside second inner loop */
                if (k % 2 == 0) {
                    total += 23;
                }
            }
        }
        
        /* Final outer loop code */
        total -= i;
    }
    
    results[2] = total;
}

/* Test case 4: Complex partial overlap with early exits */
void test_complex_overlap(int n) {
    volatile int val = 0;
    
    for (int a = 0; a < n; a++) {
        val += a * 7;
        
        /* Complex condition determining inner loop execution */
        switch (a % 4) {
            case 0:
                /* Fully contained inner loop */
                for (int b = 0; b < (a % 5) + 2; b++) {
                    val += b * 11;
                    record_value(counter++, b);
                }
                break;
                
            case 1:
                /* Partially overlapping inner loop with extra code */
                val += 13;
                for (int b = 0; b < (a % 3) + 3; b++) {
                    val += b * 17;
                    if (b % 2 == 0) {
                        val += 19;
                    }
                }
                val += 23;
                break;
                
            case 2:
                /* No inner loop, just outer loop code */
                val += 29;
                break;
                
            default:
                /* Another partially overlapping pattern */
                for (int b = 0; b < (a % 4) + 1; b++) {
                    val += b * 31;
                    record_value(counter++, b * 5);
                }
                val += 37;
                for (int c = 0; c < 2; c++) {
                    val += c * 41;
                }
                break;
        }
        
        val -= a * 3;
    }
    
    results[3] = val;
}

/* Test case 5: Loop with invariant code motion challenges */
void test_invariant_challenges(int n) {
    volatile int sum = 0;
    volatile int mod = rand() % 10 + 1;  /* Volatile to prevent hoisting */
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        sum += i * mod;  /* mod is loop-invariant but volatile */
        
        /* Inner loop that shares some blocks */
        if (i % mod == 0) {  /* Condition depends on volatile */
            for (int j = 0; j < (i % 8) + 1; j++) {
                sum += j * (mod + 1);
                record_value(counter++, j);
                
                /* Conditional creating separate basic blocks */
                if (j * mod > 10) {
                    sum += 47;
                } else {
                    sum += 53;
                }
            }
        } else {
            /* Alternative path with different loop structure */
            for (int j = 0; j < 2; j++) {
                sum += j * 59;
            }
        }
    }
    
    results[4] = sum;
}

int main() {
    /* Initialize random seed for condition functions */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; i++) {
        results[i] = 0;
    }
    
    /* Execute test cases with different parameters */
    test_partial_overlap_1(50);          /* Medium-sized loops */
    test_three_level_nesting(30);        /* Three-level nesting */
    test_sibling_loops(40);              /* Sibling loops */
    test_complex_overlap(35);            /* Complex overlap patterns */
    test_invariant_challenges(25);       /* Volatile challenges */
    
    /* Additional small test to ensure various loop bounds */
    volatile int final_check = 0;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < i + 1; j++) {
                final_check += j;
            }
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Counter: %d\n", counter);
    printf("Sample results: %d, %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    printf("Final check: %d\n", final_check);
    
    return 0;
}
