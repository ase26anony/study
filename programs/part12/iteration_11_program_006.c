/* Test program for hardware loop nesting analysis in GCC */
/* Target: hw-doloop.cc lines 429-436 */

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

/* Function with conditional inner loop - creates partial basic block overlap */
void test_partial_overlap_1(int n) {
    for (int i = 0; i < n; ++i) {          /* Outer loop L1 */
        side_effect(i);
        
        if (i % 3 == 0) {                  /* Conditional branch */
            /* Inner loop L2 - only executes sometimes */
            for (int j = 0; j < i + 5; ++j) {
                side_effect(i * 100 + j);
                if (j % 2 == 0) {          /* Nested conditional inside inner loop */
                    side_effect(j * 2);
                }
            }
        } else {
            /* Alternative path - different basic blocks not in inner loop */
            side_effect(i * 1000);
            if (i % 5 == 0) {
                side_effect(i * 2000);
            }
        }
        
        /* More code in outer loop after conditional */
        side_effect(i * 3);
    }
}

/* More complex nesting with three levels and varying overlap */
void test_three_level_nesting(int n) {
    for (int a = 0; a < n; ++a) {          /* Outer loop L3 */
        side_effect(a * 10);
        
        for (int b = 0; b < a + 3; ++b) {  /* Middle loop L4 */
            side_effect(a * 100 + b);
            
            if (b % 2 == 0) {
                /* Innermost loop L5 - fully contained in L4 */
                for (int c = 0; c < b + 2; ++c) {
                    side_effect(a * 1000 + b * 100 + c);
                    if (c % 3 == 0) {
                        side_effect(c * 50);
                    }
                }
            } else {
                /* Alternative path in L4 */
                side_effect(b * 500);
            }
        }
        
        /* Additional outer loop code not in any inner loop */
        if (a % 4 == 0) {
            side_effect(a * 3000);
        }
    }
}

/* Sibling loops inside an outer loop - creates cousin relationship */
void test_sibling_loops(int n) {
    for (int x = 0; x < n; ++x) {          /* Outer loop L6 */
        side_effect(x * 5);
        
        if (x % 3 == 0) {
            /* First inner loop L7 */
            for (int y = 0; y < x + 2; ++y) {
                side_effect(x * 100 + y * 10);
                if (y % 2 == 0) {
                    side_effect(y * 25);
                }
            }
        } else {
            /* Second inner loop L8 (sibling of L7) */
            for (int z = 0; z < x + 4; ++z) {
                side_effect(x * 200 + z * 5);
                if (z % 3 == 0) {
                    side_effect(z * 33);
                }
            }
        }
        
        /* Code that's in outer loop but not in either inner loop */
        side_effect(x * 7);
    }
}

/* Loop with early exit creating partial containment */
void test_early_exit_loop(int n) {
    for (int i = 0; i < n; ++i) {          /* Loop L9 */
        side_effect(i * 2);
        
        if (i > n / 2) {
            break;  /* Creates additional basic blocks */
        }
        
        for (int j = 0; j < i + 3; ++j) {  /* Loop L10 */
            side_effect(i * 50 + j);
            if (j == i) {
                continue;  /* More control flow variation */
            }
            side_effect(j * 10);
        }
        
        if (i % 7 == 0) {
            continue;
        }
        
        side_effect(i * 3);
    }
}

/* Mixed loop types with while and do-while */
void test_mixed_loop_types(int n) {
    int i = 0;
    while (i < n) {                        /* Loop L11 (while) */
        side_effect(i * 4);
        
        if (i % 2 == 0) {
            int j = 0;
            do {                           /* Loop L12 (do-while) */
                side_effect(i * 100 + j * 2);
                j++;
            } while (j < i + 2);
        } else {
            side_effect(i * 500);
        }
        
        i++;
        side_effect(i * 6);
    }
}

int main() {
    /* Seed random for variability but use deterministic patterns */
    srand(42);
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Test 1: Partial overlap with conditional inner loop */
    printf("Test 1: Partial overlap pattern\n");
    test_partial_overlap_1(20);
    
    /* Test 2: Three-level nesting with full and partial containment */
    printf("Test 2: Three-level nesting\n");
    test_three_level_nesting(15);
    
    /* Test 3: Sibling loops (cousin relationship) */
    printf("Test 3: Sibling loops\n");
    test_sibling_loops(12);
    
    /* Test 4: Early exit creating partial containment */
    printf("Test 4: Early exit pattern\n");
    test_early_exit_loop(25);
    
    /* Test 5: Mixed loop types */
    printf("Test 5: Mixed loop types\n");
    test_mixed_loop_types(10);
    
    /* Final checksum to ensure all loops executed */
    printf("Final checksum: %d\n", checksum);
    printf("Total operations: %d\n", counter);
    
    /* Verify some results to prevent dead code elimination */
    int verify = 0;
    for (int i = 0; i < 100 && i < counter; ++i) {
        verify += results[i];
    }
    printf("Verification sum: %d\n", verify);
    
    return 0;
}
