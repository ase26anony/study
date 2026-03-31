/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Function to create side effects and prevent dead code elimination */
static void __attribute__((noinline, noipa)) 
side_effect(int idx, int val) {
    global_array[idx % 1000] += val;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
static void __attribute__((noinline, noipa))
pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int temp1, temp2, temp3;
    
    for (int i = 0; i < iterations; i++) {
        /* Create a phi node: x is phi(0, x_prev + 1) */
        int x;
        if (i == 0) {
            x = 0;  /* Initial value */
        } else {
            x = prev + 1;  /* Value from previous iteration */
        }
        
        /* Chain of assignments to test the while loop in auto-profile.cc */
        temp1 = x;      /* GIMPLE_ASSIGN 1 */
        temp2 = temp1;  /* GIMPLE_ASSIGN 2 */
        temp3 = temp2;  /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 (RHS is 0) */
        if (temp3 == 0) {
            /* Hot path - executed frequently */
            side_effect(i, 1);
        } else {
            /* Cold path - rarely executed */
            if (temp3 == 1) {
                /* Another comparison against constant 1 */
                side_effect(i, -1);
            }
        }
        
        prev = x;  /* Store for next iteration's phi */
    }
}

/* Pattern B: Conditional merge phi */
static void __attribute__((noinline, noipa))
pattern_b_merge_phi(int iterations) {
    volatile int seed = 42;  /* Prevent constant folding */
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi at merge point of conditional */
        int val;
        if ((seed + i) % 10 == 0) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Direct comparison against constant 1 */
        if (val == 1) {
            /* Hot path */
            side_effect(i, 2);
        }
        
        /* Additional chain to obscure origin */
        int copy1 = val;
        int copy2 = copy1;
        if (copy2 == 0) {
            /* Also test comparison against 0 */
            side_effect(i, 3);
        }
    }
}

/* Pattern C: Nested loop with complex phi */
static void __attribute__((noinline, noipa))
pattern_c_nested_phi(int outer_iterations) {
    int state = 0;
    
    for (int i = 0; i < outer_iterations; i++) {
        int inner_limit = (i % 3) + 1;
        
        for (int j = 0; j < inner_limit; j++) {
            /* Phi from outer loop and previous inner iteration */
            int phi_var;
            if (j == 0) {
                phi_var = state;
            } else {
                phi_var = (phi_var + 1) % 2;
            }
            
            /* Multiple assignments chain */
            int a = phi_var;
            int b = a;
            int c = b;
            
            /* Test both 0 and 1 comparisons */
            if (c == 0) {
                side_effect(i + j, 5);
            }
            if (c == 1) {
                side_effect(i + j, 7);
            }
        }
        
        /* Update state for next outer iteration */
        state = (state + 1) % 2;
    }
}

/* Pattern D: Boolean phi from multiple conditions */
static void __attribute__((noinline, noipa))
pattern_d_bool_phi(int iterations) {
    bool flag1 = false;
    bool flag2 = true;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex boolean expression creating phi nodes */
        bool cond1 = (i % 7) == 0;
        bool cond2 = (i % 13) == 0;
        
        /* Phi merging two boolean values */
        bool result;
        if (cond1) {
            result = flag1;
        } else {
            result = flag2;
        }
        
        /* Boolean comparison (compiled to == 1 or == 0) */
        if (result == true) {
            side_effect(i, 11);
        }
        
        /* Toggle flags to create varying phi values */
        flag1 = !flag1;
        if (i % 5 == 0) {
            flag2 = !flag2;
        }
    }
}

/* Main hot function that combines all patterns */
static void __attribute__((noinline, noipa))
hot_function(int iterations) {
    /* Mix patterns to create diverse control flow */
    int quarter = iterations / 4;
    
    pattern_a_loop_phi(quarter);
    pattern_b_merge_phi(quarter);
    pattern_c_nested_phi(quarter / 10);  /* Smaller due to nested loops */
    pattern_d_bool_phi(quarter);
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Reset global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function multiple times to ensure profiling */
    for (int run = 0; run < 3; run++) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d (global_counter: %d)\n", checksum, global_counter);
    
    return 0;
}
