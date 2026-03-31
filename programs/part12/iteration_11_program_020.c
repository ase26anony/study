/* Test program for hardware loop nesting analysis in GCC */
/* Designed to trigger bitmap_intersect_compl_p logic in hw-doloop.cc */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
void side_effect(int value) {
    results[counter++ % 1000] = value;
    checksum ^= value;
}

/* Test case 1: Simple nested loops with partial overlap */
void test_partial_overlap_nested() {
    int i, j, k;
    volatile int temp = 0;
    
    /* Outer loop - will contain inner loops partially */
    for (i = 0; i < 50; i++) {
        side_effect(i);
        
        /* Conditional that creates partial overlap */
        if (i % 3 == 0) {
            /* Inner loop 1 - fully contained in this branch */
            for (j = 0; j < 20; j++) {
                temp = i * j;
                side_effect(temp);
                
                /* Deeply nested loop */
                if (j % 2 == 0) {
                    for (k = 0; k < 10; k++) {
                        side_effect(i + j + k);
                    }
                } else {
                    /* Alternative path without inner loop */
                    side_effect(i - j);
                }
            }
        } else if (i % 3 == 1) {
            /* Different inner loop structure */
            for (j = 5; j < 25; j++) {
                side_effect(i * 100 + j);
                
                /* Conditional with partial overlap */
                if (j > 15) {
                    for (k = 0; k < 5; k++) {
                        side_effect(k * 1000);
                    }
                }
            }
        } else {
            /* No inner loop in this branch - creates blocks not in inner loops */
            side_effect(i * 1000);
            temp = rand() % 100;
            side_effect(temp);
        }
    }
}

/* Test case 2: Sibling loops with shared outer loop blocks */
void test_sibling_loops() {
    int a, b, c;
    volatile int acc = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (a = 0; a < 30; a++) {
        side_effect(a * 2);
        
        /* First sibling loop */
        if (a % 4 == 0) {
            for (b = 0; b < 15; b++) {
                acc += a + b;
                side_effect(acc);
                
                /* Nested conditional with loop */
                if (b % 3 == 0) {
                    for (c = 0; c < 8; c++) {
                        side_effect(c * 100);
                    }
                }
            }
        }
        
        /* Code between sibling loops - part of outer but not inner */
        acc ^= a;
        side_effect(acc);
        
        /* Second sibling loop (different condition) */
        if (a % 4 == 2) {
            for (b = 10; b < 25; b++) {
                acc -= a * b;
                side_effect(acc);
                
                /* Different nesting pattern */
                for (c = 0; c < 3; c++) {
                    side_effect(b * c);
                }
            }
        }
        
        /* More outer loop code */
        side_effect(acc * 2);
    }
}

/* Test case 3: Complex overlapping with multiple exits */
void test_complex_overlap() {
    int x, y, z;
    volatile int val = 0;
    
    for (x = 0; x < 40; x++) {
        side_effect(x);
        
        /* Switch-like structure with different loop patterns */
        switch (x % 5) {
            case 0:
                /* Fully contained inner loop */
                for (y = 0; y < 12; y++) {
                    val = x * y;
                    side_effect(val);
                }
                break;
                
            case 1:
                /* Inner loop with early exit */
                for (y = 0; y < 18; y++) {
                    if (y > 10) break;
                    side_effect(x + y * 10);
                    
                    /* Tiny inner loop */
                    for (z = 0; z < 3; z++) {
                        side_effect(z);
                    }
                }
                break;
                
            case 2:
                /* Two sequential inner loops */
                for (y = 0; y < 8; y++) {
                    side_effect(y * 100);
                }
                for (y = 5; y < 15; y++) {
                    side_effect(y * 200);
                }
                break;
                
            default:
                /* No inner loop, just computations */
                val = x * x;
                side_effect(val);
                break;
        }
        
        /* Common outer loop code after switch */
        side_effect(val ^ x);
    }
}

/* Test case 4: Loop with invariant code motion challenges */
void test_invariant_challenges() {
    int p, q, r;
    volatile int global_var = rand() % 100;
    
    /* Outer loop with invariant that can't be easily hoisted */
    for (p = 0; p < 35; p++) {
        /* Volatile read prevents hoisting */
        volatile int invariant = global_var;
        side_effect(p + invariant);
        
        /* Inner loop that uses the "invariant" */
        if (p % 6 < 3) {
            for (q = 0; q < 12; q++) {
                side_effect(q * invariant);
                
                /* Deep nesting with condition */
                if (q % 4 == 0) {
                    for (r = 0; r < 6; r++) {
                        side_effect(r * 10000 + invariant);
                    }
                } else {
                    side_effect(q * 1000);
                }
            }
        }
        
        /* Modify the "invariant" for next iteration */
        global_var ^= p;
    }
}

int main() {
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Run all test cases to create various nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_complex_overlap();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_invariant_challenges();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    printf("Final checksum: %d\n", checksum);
    printf("Counter: %d\n", counter);
    
    return 0;
}
