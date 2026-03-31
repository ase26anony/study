/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERS 50

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_result(int idx, int val) {
    results[idx % SIZE] = val;
    checksum += val;
    counter++;
}

/* Test case 1: Outer loop with two inner loops that partially overlap */
void test_partial_overlap_nested() {
    volatile int x = 0;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (int i = 0; i < ITERS; i++) {
        /* Block A: Always executed in outer loop */
        x += i * 2;
        record_result(i, x);
        
        /* Conditional: Creates partial overlap */
        if (get_condition(i, 0) == 0) {
            /* Inner loop 1 - fully contained in this branch */
            for (int j = 0; j < i % 10 + 5; j++) {
                /* Blocks only in inner loop 1 */
                x -= j;
                record_result(j, x);
                
                /* Nested deeper loop */
                if (j % 2 == 0) {
                    for (int k = 0; k < 3; k++) {
                        x += k * i;
                        record_result(k, x);
                    }
                }
            }
        } else if (get_condition(i, 0) == 1) {
            /* Inner loop 2 - different structure, creates sibling relationship */
            for (int j = 5; j < 15; j++) {
                x += i * j;
                record_result(i + j, x);
                
                /* Conditional inside inner loop 2 */
                if (j % 3 == 0) {
                    x *= 2;
                }
            }
        } else {
            /* Alternative path without any inner loops */
            x = x / 2 + 1;
            record_result(i, x * 3);
        }
        
        /* Block B: Always executed after conditional */
        x = (x + 1) % 1000;
    }
}

/* Test case 2: Three-level nesting with varying containment */
void test_three_level_varying() {
    volatile int y = 100;
    
    /* Level 1: Outer loop */
    for (int a = 0; a < 20; a++) {
        y += a;
        
        /* Level 2: Middle loop - partially contained */
        for (int b = 0; b < (a % 5) + 3; b++) {
            y -= b;
            
            /* Conditional creating partial overlap at level 3 */
            if (b % 2 == 0) {
                /* Level 3: Inner loop - fully contained */
                for (int c = 0; c < 4; c++) {
                    y += c * a * b;
                    record_result(c, y);
                }
            } else {
                /* Alternative path at level 2 */
                y *= 2;
                record_result(b, y);
            }
            
            /* Additional block in level 2 but not in level 3 */
            y = y % 500;
        }
        
        /* Block in level 1 but not in level 2 */
        if (a % 3 == 0) {
            y = 0;
        }
    }
}

/* Test case 3: Complex sibling loops with shared parent */
void test_sibling_loops() {
    volatile int z = 0;
    
    /* Parent loop */
    for (int p = 0; p < 30; p++) {
        z += p;
        
        /* First sibling loop - executes conditionally */
        if (p % 4 == 0) {
            for (int s1 = 0; s1 < 8; s1++) {
                z += s1 * p;
                record_result(s1, z);
                
                /* Deep nesting inside sibling 1 */
                if (s1 % 3 == 0) {
                    for (int d1 = 0; d1 < 2; d1++) {
                        z -= d1;
                    }
                }
            }
        }
        
        /* Code between siblings - in parent but not in either sibling */
        z = (z + 100) % 200;
        
        /* Second sibling loop - different structure */
        if (p % 3 == 0) {
            for (int s2 = 5; s2 < 12; s2++) {
                z *= (s2 % 7) + 1;
                record_result(s2, z);
            }
        } else {
            /* Alternative path with no loop */
            z /= 2;
        }
        
        /* Final block in parent */
        record_result(p, z);
    }
}

/* Test case 4: Loop with early exit creating partial blocks */
void test_early_exit_loops() {
    volatile int w = 0;
    
    for (int i = 0; i < 40; i++) {
        w += i;
        
        /* Inner loop with early exit */
        for (int j = 0; j < 10; j++) {
            w += j;
            
            if (w > 1000) {
                /* Early exit - creates block not in all iterations */
                w = 0;
                break;
            }
            
            /* Conditional continue */
            if (j % 4 == 0) {
                continue;
            }
            
            w -= 1;
        }
        
        /* Post-inner loop code */
        if (i % 5 == 0) {
            w *= 3;
        }
    }
}

int main() {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    test_three_level_varying();
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    test_early_exit_loops();
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results array sum: %d\n", final_sum);
    printf("Total operations recorded: %d\n", counter);
    
    return 0;
}
