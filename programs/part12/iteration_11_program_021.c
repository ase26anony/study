/* test_hwloops.c - Test program for hardware loop optimization analysis */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 23) % 7;
}

/* Function with side effects */
void record_result(int idx, int val) {
    results[idx % 1000] = val;
    checksum += val;
}

/* Test case 1: Simple nested loops with partial overlap */
void test_partial_overlap_nested(void) {
    int i, j, k;
    volatile int temp = 0;
    
    /* Outer loop with conditional inner loop - creates partial overlap */
    for (i = 0; i < 50; ++i) {
        /* Code block that's in outer loop but NOT in inner loop */
        temp += i * 2;
        record_result(i, temp);
        
        /* Conditional that determines whether inner loop executes */
        if (get_condition(i, 0) > 2) {
            /* Inner loop - fully contained within this branch */
            for (j = 0; j < 30; ++j) {
                /* Code that's only in inner loop */
                temp += j * 3;
                record_result(100 + j, temp);
                
                /* Another level of nesting with different pattern */
                if (get_condition(i, j) < 4) {
                    for (k = 0; k < 10; ++k) {
                        temp += k * 5;
                        record_result(200 + k, temp);
                    }
                } else {
                    /* Alternative path in middle loop */
                    temp += 1000;
                }
            }
        } else {
            /* Alternative path in outer loop - NOT in inner loop */
            temp += i * 100;
            record_result(300 + i, temp);
        }
        
        /* More outer loop code after the conditional */
        temp -= i;
    }
}

/* Test case 2: Sibling loops inside outer loop */
void test_sibling_loops(void) {
    int i, j, k;
    volatile int acc = 0;
    
    for (i = 0; i < 40; ++i) {
        /* First inner loop (executes conditionally) */
        if (i % 3 == 0) {
            for (j = 0; j < 20; ++j) {
                acc += i * j;
                record_result(400 + j, acc);
                
                /* Deeply nested loop */
                for (k = 0; k < 5; ++k) {
                    acc += k * 7;
                    record_result(500 + k, acc);
                }
            }
        }
        
        /* Code between sibling loops */
        acc += i * 11;
        
        /* Second inner loop (different condition) */
        if (i % 4 == 1) {
            for (j = 10; j < 25; ++j) {
                acc += j * 13;
                record_result(600 + j, acc);
                
                /* Different nesting pattern */
                if (j % 2 == 0) {
                    for (k = 0; k < 8; ++k) {
                        acc += k * 17;
                        record_result(700 + k, acc);
                    }
                }
            }
        }
        
        /* More outer loop code */
        acc -= i * 2;
    }
}

/* Test case 3: Complex overlapping loop structures */
void test_complex_overlap(void) {
    int a, b, c, d;
    volatile int counter = 0;
    
    /* Level 1 loop */
    for (a = 0; a < 25; ++a) {
        counter += a;
        
        /* Level 2 loop - partially overlapping with level 3 */
        for (b = 0; b < 15; ++b) {
            counter += b * 2;
            record_result(800 + b, counter);
            
            /* Conditional level 3 loop */
            if ((a + b) % 3 == 0) {
                for (c = 0; c < 10; ++c) {
                    counter += c * 3;
                    
                    /* Level 4 loop - fully contained */
                    for (d = 0; d < 5; ++d) {
                        counter += d * 4;
                        record_result(900 + d, counter);
                    }
                    
                    /* Code in level 3 but not in level 4 */
                    if (c % 2 == 0) {
                        counter += 50;
                    }
                }
            } else {
                /* Alternative path in level 2 */
                counter += 100;
            }
            
            /* More level 2 code */
            counter -= b;
        }
        
        /* Additional level 1 code not in level 2 */
        if (a % 5 == 0) {
            for (c = 0; c < 8; ++c) {
                counter += c * 20;
                record_result(950 + c, counter);
            }
        }
    }
}

/* Test case 4: Loop with early exits and continues */
void test_loop_with_control_flow(void) {
    int i, j;
    volatile int val = 0;
    
    for (i = 0; i < 35; ++i) {
        if (i == 10) continue;  /* Skip some iterations */
        
        val += i * 3;
        
        /* Inner loop with break */
        for (j = 0; j < 20; ++j) {
            if (val > 1000) break;
            
            val += j * 7;
            record_result(1000 + j, val);
            
            /* Conditional nested loop */
            if (j % 3 == 0) {
                int k;
                for (k = 0; k < 6; ++k) {
                    val += k * 11;
                }
            } else {
                val += 33;
            }
        }
        
        if (i == 25) break;  /* Early exit from outer loop */
        val -= i;
    }
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Clear results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = 0;
    }
    checksum = 0;
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    test_complex_overlap();
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    test_loop_with_control_flow();
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final validation */
    int final_check = 0;
    for (int i = 0; i < 100; ++i) {
        final_check += results[i * 7 % 1000];
    }
    
    printf("Final validation checksum: %d\n", final_check);
    printf("All tests completed successfully.\n");
    
    return 0;
}
