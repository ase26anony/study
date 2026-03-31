/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile and global variables to prevent optimization */
volatile int g_volatile_counter = 0;
int g_results[1000] = {0};
int g_index = 0;

/* External function to create side effects */
extern int some_external_function(int x);

/* Simple function that compiler can't inline easily */
int __attribute__((noinline)) compute_value(int a, int b) {
    return (a * b) ^ (a + b);
}

/* Test case 1: Inner loop fully contained in outer loop
 * This should trigger bitmap_intersect_p but not bitmap_intersect_compl_p
 * for the inner loop relative to outer loop
 */
void test_fully_contained_nesting(void) {
    int i, j;
    int local_sum = 0;
    
    /* Outer loop with 3 basic blocks:
     * 1. Loop header (i initialization and check)
     * 2. Loop body start
     * 3. Inner loop OR conditional block
     */
    for (i = 0; i < 50; ++i) {
        /* This creates a basic block that's always executed in outer loop */
        local_sum += i * 2;
        g_volatile_counter++;
        
        /* Inner loop - fully contained within outer loop's body
         * All its basic blocks are subset of outer loop's blocks
         */
        for (j = 0; j < 25; ++j) {
            g_results[g_index++] = compute_value(i, j);
            if (j % 7 == 0) {
                /* Conditional inside inner loop creates more basic blocks */
                local_sum -= j;
            }
        }
        
        /* Additional code in outer loop after inner loop
         * This ensures outer loop has blocks not in inner loop
         */
        if (i % 3 == 0) {
            local_sum += rand() % 100;
        }
    }
    
    g_results[0] += local_sum;
}

/* Test case 2: Partially overlapping loops - inner loop in one branch of conditional
 * This should trigger bitmap_intersect_compl_p checks
 */
void test_partial_overlap_nesting(void) {
    int a, b;
    int checksum = 0;
    
    /* Outer loop */
    for (a = 0; a < 40; ++a) {
        /* Always executed in outer loop */
        checksum ^= a;
        g_volatile_counter += 2;
        
        /* Conditional that creates two separate paths
         * Only one path contains the inner loop
         */
        if (a % 4 != 0) {
            /* Path with inner loop */
            for (b = 0; b < 30; ++b) {
                g_results[g_index++] = a * 100 + b;
                if (b % 5 == 0) {
                    checksum += some_external_function(b);
                }
            }
        } else {
            /* Path without inner loop - different basic blocks
             * This creates blocks in outer loop not in inner loop
             */
            checksum -= a * 3;
            g_results[g_index++] = -a;
        }
        
        /* More code after conditional, always executed */
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    g_results[1] += checksum;
}

/* Test case 3: Three-level nesting with varying overlap patterns */
void test_three_level_nesting(void) {
    int x, y, z;
    int accumulator = 0;
    
    /* Level 1: Outer loop */
    for (x = 0; x < 20; ++x) {
        accumulator += x;
        
        /* Level 2: Middle loop - partially overlaps with outer */
        if (x % 3 != 2) {
            for (y = 0; y < 15; ++y) {
                /* Mix of operations to create multiple basic blocks */
                int temp = compute_value(x, y);
                
                /* Level 3: Innermost loop - fully contained in middle */
                if (y % 2 == 0) {
                    for (z = 0; z < 10; ++z) {
                        g_results[g_index++] = temp + z;
                        accumulator ^= z;
                    }
                } else {
                    /* Alternative path in middle loop without innermost */
                    accumulator += temp;
                    g_volatile_counter--;
                }
                
                /* More code in middle loop after conditional */
                accumulator = accumulator * 1103515245 + 12345;
            }
        } else {
            /* Alternative path in outer loop without middle loop */
            accumulator -= x * x;
            g_results[g_index++] = x * 1000;
        }
        
        /* Final code in outer loop */
        if (accumulator < 0) {
            accumulator = -accumulator;
        }
    }
    
    g_results[2] += accumulator;
}

/* Test case 4: Sibling loops inside an outer loop
 * Two inner loops that are siblings (not nested in each other)
 */
void test_sibling_loops(void) {
    int outer, inner1, inner2;
    int value = 0;
    
    for (outer = 0; outer < 30; ++outer) {
        value += outer;
        
        /* First inner loop - executes conditionally */
        if (outer % 3 == 0) {
            for (inner1 = 0; inner1 < 12; ++inner1) {
                g_results[g_index++] = value + inner1;
                value ^= inner1 * 7;
            }
        }
        
        /* Code between sibling loops */
        value = (value + 1) % 1000;
        
        /* Second inner loop - executes under different condition */
        if (outer % 4 == 1) {
            for (inner2 = 0; inner2 < 8; ++inner2) {
                g_results[g_index++] = value - inner2;
                if (inner2 % 3 == 0) {
                    value += some_external_function(inner2);
                }
            }
        } else {
            /* Alternative path without second inner loop */
            value -= outer * 5;
        }
        
        /* More outer loop code */
        g_volatile_counter += outer % 5;
    }
    
    g_results[3] += value;
}

/* Test case 5: Complex nested structure with function calls
 * Creates more complex control flow graphs
 */
void test_complex_nesting_with_calls(void) {
    int p, q, r;
    static int call_counter = 0;
    
    for (p = 0; p < 25; ++p) {
        /* Function call creates separate basic blocks */
        int base = some_external_function(p);
        
        for (q = 0; q < 18; ++q) {
            /* Conditional with early continue/break */
            if (q == p) {
                continue;
            }
            
            int product = base * q;
            
            /* Very inner loop with multiple exit points */
            for (r = 0; r < 6; ++r) {
                g_results[g_index++] = product + r + call_counter;
                
                if (r == q % 4) {
                    break;  /* Early exit from innermost loop */
                }
                
                if ((p + q + r) % 7 == 0) {
                    /* Nested conditional inside innermost loop */
                    call_counter++;
                    g_volatile_counter += r;
                }
            }
            
            /* Code after innermost loop but inside middle loop */
            if (product > 100) {
                product %= 100;
            }
        }
        
        /* Final outer loop code with another conditional */
        if (p % 6 == 0) {
            for (r = 0; r < 3; ++r) {
                /* Another small loop not nested in the middle loop */
                g_results[g_index++] = p * 1000 + r;
            }
        }
    }
    
    g_results[4] += call_counter;
}

/* Dummy implementation of external function */
int some_external_function(int x) {
    /* Use volatile to prevent optimization */
    static volatile int seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF ^ x;
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Reset global index */
    g_index = 5;  /* Leave first 5 entries for checksums */
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_fully_contained_nesting();
    printf("Test 1 completed\n");
    
    test_partial_overlap_nesting();
    printf("Test 2 completed\n");
    
    test_three_level_nesting();
    printf("Test 3 completed\n");
    
    test_sibling_loops();
    printf("Test 4 completed\n");
    
    test_complex_nesting_with_calls();
    printf("Test 5 completed\n");
    
    /* Calculate final checksum to ensure all loops executed */
    int final_checksum = 0;
    for (int i = 0; i < g_index && i < 1000; ++i) {
        final_checksum ^= g_results[i];
        final_checksum = (final_checksum << 1) | (final_checksum >> 31);
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Total results stored: %d\n", g_index);
    
    return 0;
}
