/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

/* Test case 1: Nested loops with conditional inner loop execution
 * Creates partial overlap where inner loop blocks are subset of outer loop blocks
 */
void test_partial_overlap_1(void) {
    int i, j, k;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (i = 0; i < 50; ++i) {
        /* Code block in outer loop but not in any inner loop */
        record_value(counter++, i * 3);
        
        /* Conditional that determines whether inner loop executes */
        if (get_condition(i, 0) > 2) {
            /* First inner loop - fully contained in this branch */
            for (j = 0; j < 30; ++j) {
                /* Code that's only in this inner loop */
                record_value(counter++, i * 100 + j);
                
                /* Another conditional inside inner loop */
                if (j % 3 == 0) {
                    /* Innermost loop - creates deeper nesting */
                    for (k = 0; k < 10; ++k) {
                        record_value(counter++, i * 1000 + j * 10 + k);
                    }
                } else {
                    /* Alternative path in inner loop */
                    record_value(counter++, i * 200 + j * 5);
                }
            }
        } else {
            /* Alternative branch - no inner loops here */
            record_value(counter++, i * 7 + 123);
            
            /* But has its own loop that's sibling to the one above */
            for (j = 20; j < 25; ++j) {
                record_value(counter++, i * 300 + j);
            }
        }
        
        /* More code in outer loop after the conditional */
        record_value(counter++, i * 11);
    }
}

/* Test case 2: Complex nesting with multiple inner loops at same level
 * Creates scenarios where loops are "cousins" rather than strictly nested
 */
void test_cousin_loops(void) {
    int a, b, c;
    
    for (a = 0; a < 40; ++a) {
        /* Block A1: before first inner loop */
        record_value(counter++, a * 2);
        
        if (a % 4 != 0) {
            /* First inner loop structure */
            for (b = 0; b < 15; ++b) {
                record_value(counter++, a * 100 + b * 2);
                
                /* Conditional that sometimes skips innermost */
                if (b % 5 > 2) {
                    for (c = 0; c < 8; ++c) {
                        record_value(counter++, a * 1000 + b * 10 + c);
                    }
                }
            }
        }
        
        /* Block A2: between inner loops */
        record_value(counter++, a * 3 + 456);
        
        if (a % 3 != 0) {
            /* Second inner loop - sibling/cousin to first */
            for (b = 5; b < 20; ++b) {
                record_value(counter++, a * 200 + b * 3);
                
                /* Different control flow pattern */
                switch (b % 4) {
                    case 0:
                        for (c = 0; c < 5; ++c) {
                            record_value(counter++, a * 2000 + b * 20 + c);
                        }
                        break;
                    case 1:
                        record_value(counter++, a * 1500 + b);
                        break;
                    default:
                        /* No inner loop here */
                        record_value(counter++, a * 2500 + b * 7);
                }
            }
        }
        
        /* Block A3: after second inner loop */
        record_value(counter++, a * 5 + 789);
    }
}

/* Test case 3: Loop with early exits creating complex block relationships */
void test_early_exit_nesting(void) {
    int x, y, z;
    
    for (x = 0; x < 35; ++x) {
        record_value(counter++, x * 13);
        
        /* Early exit condition */
        if (results[x % 1000] > 10000) {
            break;
        }
        
        for (y = 0; y < 12; ++y) {
            /* Conditional continue in middle loop */
            if (y == 6) continue;
            
            record_value(counter++, x * 100 + y * 7);
            
            for (z = 0; z < 6; ++z) {
                /* Innermost with break */
                if (z == 3 && (x + y) % 5 == 0) {
                    record_value(counter++, 99999);
                    break;
                }
                record_value(counter++, x * 1000 + y * 50 + z);
            }
            
            /* Code after inner loop but still in middle loop */
            if (y % 2 == 0) {
                record_value(counter++, x * 500 + y);
            }
        }
        
        /* Alternative path that sometimes executes */
        if (x % 7 == 0) {
            for (y = 3; y < 8; ++y) {
                record_value(counter++, x * 700 + y * 11);
            }
        }
    }
}

/* Test case 4: Do-while and while loops mixed with for loops */
void test_mixed_loop_types(void) {
    int p = 0;
    int q, r;
    
    /* Do-while as outer loop */
    do {
        record_value(counter++, p * 9);
        
        /* For loop inside */
        for (q = 0; q < 8; ++q) {
            record_value(counter++, p * 100 + q * 4);
            
            /* While loop as innermost */
            r = 0;
            while (r < 4) {
                record_value(counter++, p * 1000 + q * 40 + r);
                r++;
                
                if (r == 2 && q % 3 == 0) {
                    record_value(counter++, 55555);
                    /* Early exit from while */
                    break;
                }
            }
            
            /* Code after while but still in for */
            if (q % 2 == 1) {
                record_value(counter++, p * 600 + q);
            }
        }
        
        p++;
    } while (p < 25);
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    /* Initialize results array */
    for (int i = 0; i < 1000; i++) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute each test case to create different loop nesting patterns */
    test_partial_overlap_1();
    printf("Test 1 complete, checksum = %d\n", checksum);
    
    test_cousin_loops();
    printf("Test 2 complete, checksum = %d\n", checksum);
    
    test_early_exit_nesting();
    printf("Test 3 complete, checksum = %d\n", checksum);
    
    test_mixed_loop_types();
    printf("Test 4 complete, checksum = %d\n", checksum);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += results[i];
    }
    
    printf("Final array check: %d\n", final_check);
    printf("Total operations recorded: %d\n", counter);
    
    return 0;
}
