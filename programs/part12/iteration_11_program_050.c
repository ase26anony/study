/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Test case 1: Three-level nested loops with partial overlap */
void test_case_1() {
    volatile int local_sum = 0;
    
    /* Outer loop - will contain multiple inner structures */
    for (int i = 0; i < 50; ++i) {
        /* First conditional branch - contains inner loop j */
        if (get_condition(i, 0) < 4) {
            /* Middle loop - partially overlaps with outer */
            for (int j = 0; j < 30; ++j) {
                /* Innermost loop - fully contained in middle loop */
                for (int k = 0; k < 10; ++k) {
                    results[counter++ % 1000] = i * j * k;
                    local_sum += (i + j + k) & 0xFF;
                }
                
                /* Additional basic block in middle loop but not in innermost */
                if (j % 3 == 0) {
                    results[counter++ % 1000] = i ^ j;
                    local_sum += (i * j) & 0xFF;
                }
            }
        } 
        /* Second conditional branch - no inner loop, different basic blocks */
        else {
            results[counter++ % 1000] = i * 100;
            local_sum += i * 17;
            
            /* Additional computation to create more basic blocks */
            for (int x = 0; x < 5; ++x) {
                results[counter++ % 1000] = i + x;
            }
        }
        
        /* Common code after conditional - part of outer loop but not inner */
        results[counter++ % 1000] = i;
        local_sum += i & 0xFF;
    }
    
    checksum += local_sum;
}

/* Test case 2: Sibling loops inside outer loop with partial overlap */
void test_case_2() {
    volatile int local_sum = 0;
    
    for (int outer = 0; outer < 40; ++outer) {
        /* First sibling loop - executes under condition */
        if (outer % 3 == 0) {
            for (int inner1 = 0; inner1 < 20; ++inner1) {
                results[counter++ % 1000] = outer * inner1;
                local_sum += (outer + inner1) & 0xFF;
                
                /* Conditional inside inner1 creates additional basic blocks */
                if (inner1 % 2 == 0) {
                    results[counter++ % 1000] = inner1;
                }
            }
        }
        
        /* Code between sibling loops - part of outer but not inner1 */
        results[counter++ % 1000] = outer * 2;
        
        /* Second sibling loop - different condition */
        if (outer % 4 == 1) {
            for (int inner2 = 0; inner2 < 15; ++inner2) {
                results[counter++ % 1000] = outer + inner2;
                local_sum += (outer * inner2) & 0xFF;
                
                /* Different structure than inner1 */
                for (int deep = 0; deep < 3; ++deep) {
                    results[counter++ % 1000] = deep;
                }
            }
        }
        
        /* More outer loop code */
        local_sum += outer & 0x7F;
    }
    
    checksum += local_sum;
}

/* Test case 3: Complex partial overlap with early exits */
void test_case_3() {
    volatile int local_sum = 0;
    
    for (int a = 0; a < 35; ++a) {
        /* Loop that sometimes executes, sometimes doesn't */
        if (a % 5 != 0) {
            for (int b = 0; b < 25; ++b) {
                /* Early exit from inner loop creates separate basic block */
                if (b > 20 && (a + b) % 11 == 0) {
                    results[counter++ % 1000] = -1;
                    break;
                }
                
                results[counter++ % 1000] = a * 1000 + b;
                local_sum += (a + b * 3) & 0xFF;
                
                /* Very inner loop with small iteration count */
                for (int c = 0; c < 4; ++c) {
                    results[counter++ % 1000] = c;
                    local_sum += c;
                }
            }
        } else {
            /* Alternative path with its own loop structure */
            for (int b = 0; b < 10; ++b) {
                results[counter++ % 1000] = b * 100;
                local_sum += b * 7;
            }
        }
        
        /* Unconditional inner loop - always executes */
        for (int d = 0; d < 8; ++d) {
            results[counter++ % 1000] = a * 10 + d;
            local_sum += d;
        }
    }
    
    checksum += local_sum;
}

/* Test case 4: Nested loops with function calls creating complex CFG */
void helper_func(int x, int y) {
    volatile static int state = 0;
    results[counter++ % 1000] = x - y + state;
    state = (state + 1) % 100;
}

void test_case_4() {
    volatile int local_sum = 0;
    
    for (int i = 0; i < 30; ++i) {
        /* First level conditional */
        if (i % 2 == 0) {
            for (int j = 0; j < 20; ++j) {
                /* Function call creates separate basic block */
                helper_func(i, j);
                local_sum += i + j;
                
                /* Conditional inside with small loop */
                if (j % 3 == 0) {
                    for (int k = 0; k < 6; ++k) {
                        results[counter++ % 1000] = k * 100;
                        local_sum += k;
                    }
                } else {
                    results[counter++ % 1000] = j * 50;
                }
            }
        }
        
        /* Always execute this inner loop */
        for (int m = 0; m < 12; ++m) {
            results[counter++ % 1000] = i * m;
            local_sum += m;
            
            /* Nested conditional with break */
            if (m == 5) {
                results[counter++ % 1000] = 999;
                break;
            }
        }
    }
    
    checksum += local_sum;
}

int main() {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_case_1();
    printf("Test case 1 completed, checksum so far: %d\n", checksum);
    
    test_case_2();
    printf("Test case 2 completed, checksum so far: %d\n", checksum);
    
    test_case_3();
    printf("Test case 3 completed, checksum so far: %d\n", checksum);
    
    test_case_4();
    printf("Test case 4 completed, checksum so far: %d\n", checksum);
    
    /* Final verification */
    volatile int final_check = 0;
    for (int i = 0; i < 100; ++i) {
        final_check += results[i];
    }
    
    printf("Final checksum: %d, Array check: %d\n", checksum, final_check);
    printf("Counter value: %d\n", counter);
    
    return 0;
}
