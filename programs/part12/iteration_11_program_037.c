/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 * lines 429-436
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Volatile and global variables to prevent optimization */
volatile int g_volatile_counter = 0;
int g_results[SIZE] = {0};
int g_checksum = 0;

/* Function with side effects */
int get_random_limit(int base) {
    g_volatile_counter++;
    return base + (rand() % 10);
}

/* Test case 1: Inner loop fully contained in outer loop
 * This should trigger bitmap_intersect_p but not bitmap_intersect_compl_p
 * for the inner-outer relationship
 */
void test_fully_contained_nesting(void) {
    int i, j, k;
    int limit_j = get_random_limit(MID_ITER);
    int limit_k = get_random_limit(INNER_ITER);
    
    for (i = 0; i < OUTER_ITER; i++) {
        /* Outer loop has code before inner loop */
        g_results[i % SIZE] += i;
        
        for (j = 0; j < limit_j; j++) {
            /* Middle loop has its own code */
            g_volatile_counter += j;
            
            for (k = 0; k < limit_k; k++) {
                /* Innermost loop - fully contained in both outer loops */
                g_results[(i + j + k) % SIZE] ^= (i * j * k);
                g_volatile_counter++;
            }
            
            /* Middle loop continues after inner loop */
            g_results[j % SIZE] -= g_volatile_counter;
        }
        
        /* Outer loop continues after middle loop */
        if (i % 3 == 0) {
            g_volatile_counter *= 2;
        }
    }
}

/* Test case 2: Partial overlap with conditional inner loop execution
 * Inner loop executes in only one branch of conditional
 * This creates partial basic block overlap
 */
void test_partial_overlap_conditional(void) {
    int a, b, c;
    int outer_limit = get_random_limit(OUTER_ITER);
    
    for (a = 0; a < outer_limit; a++) {
        /* Always executed in outer loop */
        g_results[a % SIZE] += a * 2;
        
        if (a % 2 == 0) {
            /* Branch 1: Contains inner loop */
            int mid_limit = get_random_limit(MID_ITER);
            
            for (b = 0; b < mid_limit; b++) {
                g_volatile_counter += b;
                
                /* Another level of nesting inside this branch */
                for (c = 0; c < INNER_ITER; c++) {
                    g_results[(a + b + c) % SIZE] |= (1 << (c % 8));
                }
                
                /* Code after inner loop in middle loop */
                if (b % 4 == 0) {
                    g_volatile_counter--;
                }
            }
        } else {
            /* Branch 2: No inner loops, just different code */
            g_volatile_counter *= 3;
            g_results[a % SIZE] -= g_volatile_counter;
        }
        
        /* Common code after conditional */
        g_checksum += a;
    }
}

/* Test case 3: Sibling loops with partial overlap
 * Two inner loops at same nesting level with different conditions
 */
void test_sibling_loops_partial_overlap(void) {
    int x, y, z;
    int limit_x = get_random_limit(OUTER_ITER / 2);
    
    for (x = 0; x < limit_x; x++) {
        /* Code before first sibling loop */
        int temp = g_volatile_counter;
        
        if (x % 3 == 0) {
            /* First sibling loop */
            for (y = 0; y < MID_ITER; y++) {
                g_results[y % SIZE] += x + y;
                g_volatile_counter ^= y;
            }
        }
        
        /* Code between sibling loops */
        g_checksum += temp;
        
        if (x % 5 == 0) {
            /* Second sibling loop - different structure */
            for (z = 0; z < INNER_ITER; z++) {
                g_results[z % SIZE] *= (x + 1);
                
                /* Nested inside second sibling */
                for (y = 0; y < 5; y++) {
                    g_volatile_counter += z * y;
                }
            }
        }
        
        /* Code after sibling loops */
        g_results[x % SIZE] &= 0xFF;
    }
}

/* Test case 4: Complex nesting with mixed containment
 * Some loops fully contained, others partially overlapping
 */
void test_complex_mixed_nesting(void) {
    int p, q, r, s;
    int outer_lim = OUTER_ITER;
    
    for (p = 0; p < outer_lim; p++) {
        /* Level 1 code */
        int base = get_random_limit(10);
        
        for (q = base; q < MID_ITER; q++) {
            /* Level 2 - always executes */
            g_volatile_counter += p * q;
            
            if (q % 2 == 0) {
                /* Level 3a - partially contained */
                for (r = 0; r < INNER_ITER; r++) {
                    g_results[r % SIZE] += p + q + r;
                }
            } else {
                /* Level 3b - different structure with deeper nesting */
                for (s = 0; s < 8; s++) {
                    g_checksum += s;
                    
                    /* Level 4 - fully contained in 3b */
                    for (r = 0; r < 5; r++) {
                        g_results[(p + s + r) % SIZE] ^= (q * r);
                    }
                }
            }
            
            /* More level 2 code after conditional */
            g_volatile_counter -= q;
        }
        
        /* Final outer loop code */
        if (p % 7 == 0) {
            g_results[p % SIZE] = g_volatile_counter;
        }
    }
}

/* Test case 5: Loop with early exit creating partial blocks */
void test_early_exit_partial(void) {
    int i, j;
    int limit_i = get_random_limit(OUTER_ITER);
    
    for (i = 0; i < limit_i; i++) {
        g_results[i % SIZE] = i;
        
        /* Inner loop with early exit condition */
        for (j = 0; j < MID_ITER; j++) {
            g_volatile_counter += j;
            
            if (g_volatile_counter > 1000) {
                /* Early exit from inner loop only */
                break;
            }
            
            g_results[(i + j) % SIZE] += g_volatile_counter;
        }
        
        /* This code executes even if inner loop broke early */
        g_checksum += i * 2;
        
        /* Another conditional inner loop */
        if (i % 4 == 0) {
            for (j = 5; j < INNER_ITER; j++) {
                g_volatile_counter -= j;
            }
        }
    }
}

int main(void) {
    /* Initialize random seed for varying loop bounds */
    srand(time(NULL));
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Execute each test case to create different loop nesting patterns */
    test_fully_contained_nesting();
    printf("Completed test 1\n");
    
    test_partial_overlap_conditional();
    printf("Completed test 2\n");
    
    test_sibling_loops_partial_overlap();
    printf("Completed test 3\n");
    
    test_complex_mixed_nesting();
    printf("Completed test 4\n");
    
    test_early_exit_partial();
    printf("Completed test 5\n");
    
    /* Calculate final checksum to ensure all loops executed */
    int final_sum = g_checksum;
    for (int i = 0; i < SIZE; i++) {
        final_sum += g_results[i];
    }
    final_sum += g_volatile_counter;
    
    printf("Final checksum: %d\n", final_sum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Return non-zero if something seems wrong (all zeros) */
    if (final_sum == 0 && g_volatile_counter == 0) {
        return 1;
    }
    
    return 0;
}
