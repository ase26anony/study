/* Test program for hardware loop nesting analysis in GCC */
/* Designed to trigger bitmap_intersect_compl_p checks in hw-doloop.cc */

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

/* Test case 1: Simple nested loops with partial overlap */
void test_partial_overlap_nested(void) {
    int i, j;
    int local_buffer[100];
    
    /* Outer loop with conditional inner loop */
    for (i = 0; i < 50; ++i) {
        /* Code block that's always executed in outer loop */
        local_buffer[i] = i * 2;
        g_volatile_counter++;
        
        /* Conditional that creates partial overlap */
        if (i % 3 == 0) {
            /* Inner loop - only executes sometimes */
            for (j = 0; j < 20; ++j) {
                g_results[g_index++] = compute_value(i, j);
                if (j % 5 == 0) {
                    /* Additional conditional inside inner loop */
                    g_volatile_counter += 2;
                }
            }
        } else {
            /* Alternative path in outer loop - NOT in inner loop */
            /* This creates blocks in outer loop that aren't in inner loop */
            g_results[g_index++] = i * 1000;
            some_external_function(i);
        }
        
        /* More outer loop code after the conditional */
        if (i % 7 == 0) {
            g_volatile_counter--;
        }
    }
}

/* Test case 2: Three-level nesting with complex overlap patterns */
void test_three_level_nesting(void) {
    int a, b, c;
    int temp = 0;
    
    /* Level 1: Outermost loop */
    for (a = 0; a < 30; ++a) {
        /* Some outer loop computation */
        temp = a * a;
        g_volatile_counter += temp;
        
        /* Level 2: Middle loop - sometimes executed */
        if (a % 2 == 0) {
            for (b = 0; b < 25; ++b) {
                /* Middle loop body */
                g_results[g_index++] = a + b;
                
                /* Level 3: Innermost loop - conditional */
                if (b % 3 == 0) {
                    for (c = 0; c < 15; ++c) {
                        /* Innermost loop body */
                        g_results[g_index++] = compute_value(a, compute_value(b, c));
                        g_volatile_counter += c;
                    }
                } else {
                    /* Alternative path in middle loop */
                    g_results[g_index++] = b * 100;
                }
                
                /* More middle loop code */
                if (b % 4 == 0) {
                    g_volatile_counter--;
                }
            }
        } else {
            /* Alternative path in outermost loop */
            for (b = 0; b < 10; ++b) {
                /* Different inner loop - sibling to the one above */
                g_results[g_index++] = a - b;
                g_volatile_counter += b;
            }
        }
        
        /* Final outer loop code */
        g_results[g_index++] = temp;
    }
}

/* Test case 3: Sibling loops inside outer loop */
void test_sibling_loops(void) {
    int x, y, z;
    
    /* Outer loop */
    for (x = 0; x < 40; ++x) {
        /* First inner loop (sibling 1) */
        if (x % 4 == 0) {
            for (y = 0; y < 18; ++y) {
                g_results[g_index++] = x * y;
                g_volatile_counter += y;
                
                /* Nested inside sibling 1 */
                if (y % 3 == 0) {
                    for (z = 0; z < 12; ++z) {
                        g_results[g_index++] = compute_value(x, compute_value(y, z));
                    }
                }
            }
        }
        
        /* Code between sibling loops */
        g_results[g_index++] = x * 100;
        
        /* Second inner loop (sibling 2) */
        if (x % 5 == 1) {
            for (y = 10; y < 22; ++y) {
                g_results[g_index++] = x + y;
                g_volatile_counter -= y % 3;
                
                /* Different control flow than sibling 1 */
                if (y % 4 == 0) {
                    g_results[g_index++] = y * 200;
                } else {
                    for (z = 5; z < 10; ++z) {
                        g_results[g_index++] = x * y * z;
                    }
                }
            }
        }
        
        /* More outer loop code */
        if (x % 6 == 0) {
            g_volatile_counter += 2;
        }
    }
}

/* Test case 4: Loop with early exit creating partial blocks */
void test_loop_with_early_exit(void) {
    int i, j;
    
    for (i = 0; i < 35; ++i) {
        /* Early exit condition */
        if (g_volatile_counter > 1000) {
            break;
        }
        
        /* Inner loop with its own early exit */
        for (j = 0; j < 28; ++j) {
            g_results[g_index++] = i * j;
            
            if (j > 20 && i > 15) {
                break;
            }
            
            /* Conditional code inside inner loop */
            if (j % 6 == 0) {
                g_volatile_counter += i;
            } else {
                g_volatile_counter -= j;
            }
        }
        
        /* Code that executes after inner loop completes */
        g_results[g_index++] = i * 1000;
    }
}

/* Test case 5: Mixed nested and sequential loops */
void test_mixed_loop_patterns(void) {
    int p, q, r;
    
    /* First outer loop */
    for (p = 0; p < 20; ++p) {
        /* Inner loop A */
        for (q = 0; q < 15; ++q) {
            g_results[g_index++] = p * 10 + q;
            g_volatile_counter += q % 3;
        }
        
        /* Code between inner loops */
        g_results[g_index++] = p * 100;
        
        /* Inner loop B (not nested in A, but both in outer loop) */
        if (p % 3 == 0) {
            for (r = 0; r < 12; ++r) {
                g_results[g_index++] = compute_value(p, r);
                
                /* Deeply nested inside B */
                if (r % 2 == 0) {
                    int s;
                    for (s = 0; s < 8; ++s) {
                        g_results[g_index++] = p * r * s;
                        g_volatile_counter += s;
                    }
                }
            }
        }
    }
    
    /* Sequential loop (not nested in the first) */
    for (p = 0; p < 25; ++p) {
        g_results[g_index++] = p * 50;
    }
}

/* Dummy external function */
int some_external_function(int x) {
    return x ^ 0x55;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize global array with some values */
    for (int i = 0; i < 1000; i++) {
        g_results[i] = rand() % 100;
    }
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete, g_index = %d\n", g_index);
    
    test_three_level_nesting();
    printf("Test 2 complete, g_index = %d\n", g_index);
    
    test_sibling_loops();
    printf("Test 3 complete, g_index = %d\n", g_index);
    
    test_loop_with_early_exit();
    printf("Test 4 complete, g_index = %d\n", g_index);
    
    test_mixed_loop_patterns();
    printf("Test 5 complete, g_index = %d\n", g_index);
    
    /* Compute a simple checksum to ensure loops executed */
    int checksum = 0;
    for (int i = 0; i < g_index && i < 1000; i++) {
        checksum ^= g_results[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Total iterations: %d\n", g_index);
    
    return 0;
}
