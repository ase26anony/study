/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p checks in hw-doloop.cc
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

/* Test case 1: Three-level nested loops with partial overlap */
void test_case_1() {
    volatile int a = 0, b = 0, c = 0;
    
    /* Outer loop - will contain partial blocks of inner loops */
    for (int i = 0; i < 50; ++i) {
        /* Code executed in outer loop but not in inner loops */
        record_value(counter++, i);
        
        /* Conditional that creates separate basic blocks */
        if (get_condition(i, 0) < 4) {
            /* First inner loop - fully contained within this branch */
            for (int j = 0; j < 30; ++j) {
                /* Code only in inner loop */
                a = i * j;
                record_value(counter++, a);
                
                /* Another conditional inside inner loop */
                if (get_condition(i, j) > 2) {
                    /* Innermost loop - partially overlaps with middle loop */
                    for (int k = 0; k < 20; ++k) {
                        b = i + j + k;
                        record_value(counter++, b);
                        
                        /* More conditions to create additional blocks */
                        if (k % 3 == 0) {
                            c = b * 2;
                            results[k % 1000] = c;
                        } else {
                            results[(k + 1) % 1000] = b;
                        }
                    }
                } else {
                    /* Alternative path in middle loop without innermost loop */
                    results[j % 1000] = i - j;
                }
            }
        } else {
            /* Alternative outer loop path - no inner loops here */
            results[i % 1000] = i * 100;
            /* Function call to create separate basic block */
            record_value(counter++, i * 3);
        }
        
        /* More code in outer loop after the conditional */
        checksum += i;
    }
}

/* Test case 2: Sibling loops with partial overlap in outer loop */
void test_case_2() {
    volatile int x = 0, y = 0;
    
    for (int outer = 0; outer < 40; ++outer) {
        /* Pre-loop code in outer */
        x = outer * 2;
        record_value(counter++, x);
        
        /* First sibling loop - conditionally executed */
        if (outer % 3 == 0) {
            for (int inner1 = 0; inner1 < 25; ++inner1) {
                y = outer + inner1;
                results[inner1 % 1000] = y;
                
                /* Conditional inside first sibling */
                if (inner1 % 4 == 0) {
                    record_value(counter++, y * 2);
                }
            }
        }
        
        /* Code between sibling loops */
        checksum ^= outer;
        
        /* Second sibling loop - different condition */
        if (outer % 5 != 0) {
            for (int inner2 = 0; inner2 < 20; ++inner2) {
                x = outer * inner2;
                results[(outer + inner2) % 1000] = x;
                
                /* Different structure than first sibling */
                if (inner2 > 10) {
                    record_value(counter++, x / 2);
                } else {
                    record_value(counter++, x);
                }
            }
        }
        
        /* Post-loop code in outer */
        results[outer % 1000] = checksum;
    }
}

/* Test case 3: Complex nested structure with multiple exits */
void test_case_3() {
    volatile int acc = 0;
    
    for (int i = 0; i < 60; ++i) {
        /* Early continue creates separate basic block */
        if (i % 7 == 0) {
            continue;
        }
        
        record_value(counter++, i);
        
        for (int j = 0; j < 35; ++j) {
            /* Break in middle loop creates more blocks */
            if (j > 30) {
                break;
            }
            
            acc = i * 100 + j;
            
            /* Innermost with partial containment */
            if (acc % 2 == 0) {
                for (int k = 0; k < 15; ++k) {
                    results[(i + j + k) % 1000] = acc + k;
                    
                    /* Conditional return-like behavior */
                    if (k == 10) {
                        goto inner_exit;
                    }
                }
                inner_exit:
                checksum += 1;
            } else {
                /* Alternative path without innermost */
                results[j % 1000] = acc;
            }
            
            /* More code in middle loop after conditional */
            if (j % 3 == 0) {
                record_value(counter++, j);
            }
        }
        
        /* Code only in outer loop after middle loop */
        acc += i;
    }
}

/* Test case 4: Loop with switch statement creating multiple blocks */
void test_case_4() {
    volatile int val = 0;
    
    for (int i = 0; i < 45; ++i) {
        switch (i % 4) {
            case 0:
                /* Contains inner loop */
                for (int j = 0; j < 18; ++j) {
                    val = i + j * 2;
                    record_value(counter++, val);
                }
                break;
            case 1:
                /* Different inner loop structure */
                for (int j = 5; j < 22; ++j) {
                    val = i * j;
                    results[j % 1000] = val;
                }
                break;
            case 2:
                /* No inner loop, just assignments */
                val = i * 100;
                record_value(counter++, val);
                break;
            default:
                /* Another inner loop variant */
                for (int j = 0; j < 12; ++j) {
                    if (j % 2 == 0) {
                        val = i - j;
                    } else {
                        val = i + j;
                    }
                    results[(i + j) % 1000] = val;
                }
                break;
        }
        
        /* Common outer loop code */
        checksum += val;
    }
}

int main() {
    /* Initialize random seed for condition functions */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_case_1();
    printf("Test case 1 completed. Checksum: %d\n", checksum);
    
    test_case_2();
    printf("Test case 2 completed. Checksum: %d\n", checksum);
    
    test_case_3();
    printf("Test case 3 completed. Checksum: %d\n", checksum);
    
    test_case_4();
    printf("Test case 4 completed. Checksum: %d\n", checksum);
    
    /* Final computation to ensure all loops have side effects */
    volatile int final = 0;
    for (int i = 0; i < 1000; ++i) {
        final += results[i];
    }
    
    printf("Final result: %d (counter: %d)\n", final, counter);
    
    return 0;
}
