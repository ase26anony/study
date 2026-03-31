/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 * lines 429-436
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define NEST_LEVELS 3

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int outer_counter = 0;

/* Function to create side effects that can't be optimized away */
void side_effect(int value) {
    results[value % SIZE] ^= value;
    checksum += value;
}

/* Function with conditional inner loops creating partial block overlap */
void test_partial_overlap_nesting(int n) {
    volatile int x = 0;
    
    /* Outer loop - level 1 */
    for (int i = 0; i < n; i++) {
        side_effect(i);
        outer_counter++;
        
        /* Conditional block that creates partial basic block overlap */
        if (i % 3 == 0) {
            /* Inner loop - level 2 (executed only when i % 3 == 0) */
            for (int j = 0; j < i + 1; j++) {
                side_effect(j * 2);
                
                /* Another conditional inside inner loop */
                if (j % 2 == 0) {
                    /* Innermost loop - level 3 */
                    for (int k = 0; k < 5; k++) {
                        side_effect(k * 3);
                        results[k] = i + j + k;
                    }
                } else {
                    /* Alternative path without innermost loop */
                    side_effect(j * 100);
                    results[j % 10] = i * j;
                }
            }
        } else if (i % 3 == 1) {
            /* Different inner loop structure - sibling to the above */
            for (int j = i; j > 0; j--) {
                side_effect(j * 3);
                /* This creates different basic block pattern */
                if (j > i / 2) {
                    results[j] = i * 1000 + j;
                }
            }
        } else {
            /* No inner loop here - just side effects */
            side_effect(i * 10000);
            results[i % SIZE] = i * i;
        }
    }
}

/* Test case with two inner loops at same nesting level (sibling loops) */
void test_sibling_inner_loops(int n) {
    volatile int a = 0, b = 0;
    
    for (int i = 0; i < n; i++) {
        side_effect(i + 1000);
        
        /* First inner loop under condition */
        if (i % 4 == 0) {
            for (int j = 0; j < 8; j++) {
                a += i * j;
                side_effect(a);
                results[j] += a;
            }
        }
        
        /* Some code between sibling loops */
        b = i * 2;
        side_effect(b);
        
        /* Second inner loop under different condition */
        if (i % 4 == 2) {
            for (int k = 5; k > 0; k--) {
                b -= k;
                side_effect(b);
                results[k + 10] += b;
            }
        }
        
        /* Code after second inner loop */
        results[i % 20] = a + b;
    }
}

/* Test with loop-invariant code mixed with nested loops */
void test_mixed_invariant_nested(int n) {
    volatile int invariant = rand() % 100;
    volatile int temp = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Loop-invariant computation that can't be hoisted due to volatile */
        temp = invariant * i + results[i % SIZE];
        side_effect(temp);
        
        /* Complex conditional with nested loops */
        switch (i % 5) {
            case 0:
                /* Fully contained inner loop */
                for (int j = 0; j < 3; j++) {
                    side_effect(j + i * 10);
                }
                break;
            case 1:
                /* Partially overlapping loop structure */
                for (int j = i; j < i + 3; j++) {
                    if (j % 2 == 0) {
                        side_effect(j * 100);
                    } else {
                        /* Extra basic block not in other cases */
                        results[j % SIZE] = j;
                    }
                }
                break;
            case 2:
                /* Deeper nesting */
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        side_effect(i + j + k);
                    }
                }
                break;
            default:
                /* No inner loop, different basic blocks */
                side_effect(i * 1000);
                break;
        }
        
        /* More code after the switch */
        results[(i + 1) % SIZE] = temp;
    }
}

/* Test with early exits and continues creating complex CFG */
void test_complex_control_flow(int n) {
    volatile int counter = 0;
    
    for (int i = 0; i < n; i++) {
        if (i % 7 == 0) {
            continue;  /* Creates additional basic blocks */
        }
        
        side_effect(i);
        
        /* Inner loop with its own control flow */
        for (int j = 0; j < i % 10; j++) {
            if (j == 3) {
                break;  /* More CFG complexity */
            }
            
            side_effect(i * 100 + j);
            
            /* Yet another conditional level */
            if (j % 2 == 0) {
                for (int k = 0; k < 2; k++) {
                    counter++;
                    results[counter % SIZE] = i + j + k;
                }
            }
        }
        
        if (i == n - 1) {
            break;  /* Early exit from outer loop */
        }
        
        /* Final side effect */
        side_effect(counter);
    }
}

int main() {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Test 1: Partial overlap nesting (primary target) */
    test_partial_overlap_nesting(20);
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    /* Test 2: Sibling inner loops */
    test_sibling_inner_loops(15);
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    /* Test 3: Mixed invariant code with nested loops */
    test_mixed_invariant_nested(25);
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    /* Test 4: Complex control flow with early exits */
    test_complex_control_flow(30);
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum: %d\n", final_sum);
    printf("Outer counter: %d\n", outer_counter);
    
    return 0;
}
