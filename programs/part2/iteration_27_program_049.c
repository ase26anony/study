/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int hot_function_phi_loop(int iterations, int start_val) {
    int result = 0;
    int prev_x = 0;
    
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : prev_x + 1 */
        int x = (i == 0) ? start_val : prev_x + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        /* Pattern C: Chained copies */
        int a = x;      /* First copy */
        int b = a;      /* Second copy */
        int c = b;      /* Third copy */
        
        /* Comparison against constant 0 - this should trigger the uncovered code */
        if (c == 0) {   /* RHS is constant 0 */
            /* Hot path - executed many times */
            global_array[i & 255] += 1;
            result += 1;
        } else {
            /* Cold path - rarely executed */
            global_counter += 1;
        }
        
        prev_x = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_merge_phi(int flag) {
    /* Pattern B: Merge point phi feeding comparison with 1 */
    int val;
    
    /* This creates a phi node at the merge point */
    if (flag) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Chain assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Comparison against constant 1 */
    if (tmp2 == 1) {   /* RHS is constant 1 */
        return 100;
    }
    return 0;
}

__attribute__((noinline, noipa))
int hot_function_bool_phi(int a, int b) {
    /* Boolean phi node - comparisons with true/false become 1/0 */
    bool condition = (a > b);
    
    /* Multiple assignments to create SSA chain */
    bool cond1 = condition;
    bool cond2 = cond1;
    
    /* This becomes comparison with 1 (true) */
    if (cond2) {   /* Implicit comparison with 1 */
        return a;
    }
    return b;
}

__attribute__((noinline, noipa))
int hot_function_mixed_types(int seed) {
    /* Mix integer types to create distinct SSA names */
    char c = (seed & 1) ? 0 : 1;
    short s = c;
    int i = s;
    
    /* Chain with different types */
    int j = i;
    
    /* Comparison with 0 */
    if (j == 0) {
        return 1;
    }
    return 0;
}

__attribute__((noinline, noipa))
void complex_control_flow(int n) {
    int x = 0;
    int y = 1;
    
    for (int i = 0; i < n; i++) {
        /* Create phi nodes from multiple predecessors */
        int phi_var;
        if (i & 1) {
            phi_var = x;
            x = y + 1;
        } else {
            phi_var = y;
            y = x + 1;
        }
        
        /* Multiple assignment chain */
        int a = phi_var;
        int b = a;
        int c = b;
        
        /* Alternate between comparisons with 0 and 1 */
        if ((i & 2) == 0) {
            if (c == 0) {   /* Comparison with 0 */
                global_array[i & 255] += 1;
            }
        } else {
            if (c == 1) {   /* Comparison with 1 */
                global_counter += 1;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Execute hot functions to generate profile data */
    int total = 0;
    
    /* Pattern A: Loop phi with comparison to 0 */
    total += hot_function_phi_loop(iterations, 0);
    
    /* Pattern B: Merge point phi with comparison to 1 */
    for (int i = 0; i < iterations / 100; i++) {
        total += hot_function_merge_phi(i & 1);
    }
    
    /* Boolean phi pattern */
    for (int i = 0; i < iterations / 50; i++) {
        total += hot_function_bool_phi(i, i * 2);
    }
    
    /* Mixed types pattern */
    for (int i = 0; i < iterations / 200; i++) {
        total += hot_function_mixed_types(i);
    }
    
    /* Complex control flow */
    complex_control_flow(iterations / 10);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = total;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
