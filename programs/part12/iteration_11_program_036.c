/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects that can't be optimized away */
void side_effect(int value) {
    results[counter++ % SIZE] = value;
    checksum ^= value;
}

/* Function with conditional inner loop - creates partial basic block overlap */
void nested_loops_partial_overlap(int outer_limit, int mid_limit, int inner_limit) {
    volatile int x = 0;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Code block BEFORE inner loop - creates basic blocks not in inner loop */
        side_effect(i * 2);
        
        if (i % 3 == 0) {
            /* First conditional path: contains middle loop */
            for (int j = 0; j < mid_limit; j++) {
                /* Code inside middle loop but outside inner loop */
                x = i + j;
                side_effect(x);
                
                if (j % 2 == 0) {
                    /* Innermost loop - fully contained in middle loop */
                    for (int k = 0; k < inner_limit; k++) {
                        side_effect(i * j * k);
                        x += k;
                    }
                } else {
                    /* Alternative path in middle loop - no inner loop */
                    side_effect(i - j);
                    x -= j;
                }
                
                /* More code after if-else in middle loop */
                side_effect(x * 2);
            }
        } else if (i % 3 == 1) {
            /* Second conditional path: different loop structure */
            for (int j = mid_limit; j > 0; j--) {
                side_effect(i * 100 + j);
                
                /* Partial inner loop - only runs sometimes */
                if (rand() % 2) {
                    for (int k = 0; k < inner_limit / 2; k++) {
                        side_effect(k * 7);
                    }
                }
            }
        } else {
            /* Third path: no loops at all */
            side_effect(i * 3);
            x = i * i;
        }
        
        /* Code block AFTER conditional - creates more non-overlapping blocks */
        side_effect(x + i);
    }
}

/* Function with sibling inner loops - both partially overlapping with outer */
void sibling_loops_partial_overlap(int outer_limit) {
    volatile int a = 0, b = 0;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Pre-loop code */
        a = rand() % 10;
        
        if (a < 5) {
            /* First sibling loop */
            for (int j = 0; j < i + 5; j++) {
                side_effect(j * 11);
                b += j;
                
                /* Conditional to create partial blocks */
                if (j % 3 == 0) {
                    side_effect(b * 2);
                }
            }
        } else {
            /* Second sibling loop (different structure) */
            for (int j = 10; j > 0; j--) {
                side_effect(j * 13);
                b -= j;
                
                /* Nested conditional with small inner loop */
                if (j % 4 == 0) {
                    for (int k = 0; k < 3; k++) {
                        side_effect(k * 17);
                    }
                }
            }
        }
        
        /* Post-loop code */
        side_effect(a + b);
    }
}

/* Function with three-level nesting where middle loop is NOT fully contained */
void three_level_complex_nesting(int limit1, int limit2, int limit3) {
    volatile int x = 0;
    
    /* Outer loop */
    for (int i = 0; i < limit1; i++) {
        side_effect(i);
        
        /* Middle loop - partially overlaps with outer */
        if (i % 2 == 0) {
            for (int j = 0; j < limit2; j++) {
                /* Code that's in middle but not in inner */
                x = i * j;
                side_effect(x);
                
                /* Inner loop - fully contained in middle */
                if (j % 3 == 0) {
                    for (int k = 0; k < limit3; k++) {
                        side_effect(i + j + k);
                        x += k;
                    }
                } else {
                    /* Alternative path in middle loop */
                    side_effect(j * 7);
                }
                
                /* More middle loop code */
                side_effect(x * 3);
            }
        } else {
            /* Outer loop path without middle loop */
            side_effect(i * 100);
        }
        
        /* More outer loop code */
        side_effect(x + i * 2);
    }
}

/* Function with loops that share some basic blocks but not all */
void overlapping_cousin_loops(int iterations) {
    volatile int shared = 0;
    
    /* First loop */
    for (int i = 0; i < iterations; i++) {
        /* Shared basic block */
        shared = i * 2;
        side_effect(shared);
        
        /* Unique to first loop */
        if (i % 4 == 0) {
            side_effect(i * i);
        }
        
        /* More shared code */
        side_effect(shared + 1);
    }
    
    /* Second loop with overlapping but not identical structure */
    for (int i = iterations - 1; i >= 0; i--) {
        /* Shared basic block (same operation, different context) */
        shared = i * 2;
        side_effect(shared);
        
        /* Unique to second loop */
        if (i % 5 == 0) {
            for (int j = 0; j < 3; j++) {
                side_effect(j * 19);
            }
        }
        
        /* More shared code */
        side_effect(shared - 1);
    }
}

int main() {
    /* Seed random number generator for unpredictable branches */
    srand(time(NULL));
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Test 1: Nested loops with partial basic block overlap */
    printf("Test 1: Nested loops with partial overlap\n");
    nested_loops_partial_overlap(OUTER_ITER, MID_ITER, INNER_ITER);
    
    /* Test 2: Sibling loops inside outer loop */
    printf("Test 2: Sibling loops with partial overlap\n");
    sibling_loops_partial_overlap(OUTER_ITER);
    
    /* Test 3: Three-level complex nesting */
    printf("Test 3: Three-level complex nesting\n");
    three_level_complex_nesting(OUTER_ITER / 2, MID_ITER / 2, INNER_ITER / 2);
    
    /* Test 4: Sequential loops with overlapping basic blocks */
    printf("Test 4: Overlapping cousin loops\n");
    overlapping_cousin_loops(MID_ITER);
    
    /* Additional test: Mix of all patterns */
    printf("Test 5: Mixed loop patterns\n");
    for (int pattern = 0; pattern < 4; pattern++) {
        switch (pattern % 3) {
            case 0:
                nested_loops_partial_overlap(10, 8, 5);
                break;
            case 1:
                sibling_loops_partial_overlap(12);
                break;
            case 2:
                three_level_complex_nesting(8, 6, 4);
                break;
        }
        
        /* Small loop with compile-time constant bound */
        for (int i = 0; i < 7; i++) {
            side_effect(i * 23);
        }
    }
    
    /* Final checksum calculation to ensure all loops executed */
    int final_checksum = checksum;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += results[i];
    }
    
    printf("Test completed. Final checksum: %d\n", final_checksum);
    printf("Counter value: %d\n", counter);
    
    return 0;
}
