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

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 3 + j * 7) % 5;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % SIZE] = val;
    checksum ^= val;
}

/* Test case 1: Classic nested loops with partial overlap */
void test_partial_overlap_nested(void) {
    volatile int x = 0;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Code block BEFORE inner loop - not in inner loop's bitmap */
        x += i * 2;
        record_value(i, x);
        
        /* Conditional that determines whether inner loop executes */
        if (get_condition(i, 0) > 1) {
            /* Middle loop - partially overlaps with outer */
            for (int j = 0; j < MID_ITER; j++) {
                /* Code block inside middle but before innermost */
                x -= j;
                
                /* Another conditional creating partial overlap */
                if ((i + j) % 3 == 0) {
                    /* Innermost loop - fully contained in middle */
                    for (int k = 0; k < INNER_ITER; k++) {
                        x += i * j * k;
                        counter++;
                    }
                } else {
                    /* Alternative path in middle loop */
                    x += i * 100;
                }
                
                /* Code after innermost in middle loop */
                record_value(j, x);
            }
        } else {
            /* Alternative path in outer loop - creates blocks not in inner */
            x *= 3;
            record_value(i + 100, x);
        }
        
        /* Code block AFTER inner loop in outer - not in inner's bitmap */
        x %= 1000;
    }
    
    results[0] = x;
}

/* Test case 2: Sibling loops with partial overlap in outer */
void test_sibling_loops(void) {
    volatile int y = 0;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        /* First inner loop under condition */
        if (i % 4 == 0) {
            for (int j = 0; j < MID_ITER; j++) {
                y += i * j;
                /* Small fully-contained inner loop */
                for (int k = 0; k < 5; k++) {
                    y -= k;
                }
            }
        }
        /* Code between sibling loops */
        y += rand() % 10;
        
        /* Second inner loop (sibling of first) under different condition */
        if (i % 3 == 0) {
            for (int j = 10; j < MID_ITER + 10; j++) {
                y += i + j;
                /* Different control flow inside */
                if (j % 2 == 0) {
                    y *= 2;
                }
            }
        }
        
        /* More outer-only code */
        record_value(i, y);
    }
    
    results[1] = y;
}

/* Test case 3: Complex nesting with multiple exit points */
void test_complex_nesting(void) {
    volatile int z = 0;
    int early_exit = 0;
    
    /* Outer loop with early exit possibility */
    for (int i = 0; i < OUTER_ITER && !early_exit; i++) {
        /* Middle loop 1 */
        for (int j = 0; j < i + 5; j++) {
            /* Partial overlap: some blocks not in innermost */
            if (j % 2 == 0) {
                /* Innermost loop 1 */
                for (int k = 0; k < INNER_ITER; k++) {
                    z += (i * j * k) % 100;
                    if (z > 10000) {
                        early_exit = 1;
                        break;
                    }
                }
                if (early_exit) break;
            } else {
                /* Alternative path in middle loop */
                z -= j * 10;
            }
        }
        if (early_exit) break;
        
        /* Middle loop 2 (different structure) */
        for (int j = MID_ITER; j > 0; j--) {
            z += j;
            /* Innermost loop 2 with different bounds */
            for (int k = 0; k < j && k < 10; k++) {
                z ^= k;
                counter++;
            }
        }
    }
    
    results[2] = z;
}

/* Test case 4: Loop with switch statement creating complex CFG */
void test_loop_with_switch(void) {
    volatile int w = 0;
    
    for (int i = 0; i < OUTER_ITER; i++) {
        switch (i % 4) {
            case 0:
                /* Inner loop in case 0 */
                for (int j = 0; j < 10; j++) {
                    w += i + j;
                    /* Tiny inner-inner loop */
                    for (int k = 0; k < 3; k++) {
                        w -= k;
                    }
                }
                break;
            case 1:
                /* Different inner loop structure */
                for (int j = 5; j < 15; j++) {
                    w *= (i % j) + 1;
                }
                break;
            case 2:
                /* No inner loop, just computation */
                w += i * 100;
                break;
            case 3:
                /* Nested loops with break */
                for (int j = 0; j < 8; j++) {
                    if (j == 4) break;
                    for (int k = 0; k < 6; k++) {
                        w += i * j * k;
                    }
                }
                break;
        }
        
        /* Common outer loop code */
        record_value(i + 200, w);
    }
    
    results[3] = w;
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    /* Clear results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    test_loop_with_switch();
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final computation using results to prevent dead code elimination */
    volatile int final = 0;
    for (int i = 0; i < SIZE; i++) {
        final += results[i];
    }
    
    printf("Final result: %d (counter: %d)\n", final, counter);
    printf("All tests completed successfully.\n");
    
    return 0;
}
