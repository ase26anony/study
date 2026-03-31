/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int result_array[1000] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {   /* This should trigger the uncovered code */
            global_counter += 1;
            result_array[i % 1000] = 1;
        } else {
            result_array[i % 1000] = 0;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int flag) {
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
    
    /* Comparison with 1 */
    if (tmp2 == 1) {   /* Should trigger uncovered code */
        global_counter += 2;
        result_array[global_counter % 1000] = val;
    }
}

__attribute__((noinline, noipa))
void hot_function_bool_phi(int a, int b) {
    /* Pattern C: Boolean phi node */
    bool condition;
    
    /* Create phi from comparison results */
    if (a > b) {
        condition = true;   /* becomes 1 */
    } else {
        condition = false;  /* becomes 0 */
    }
    
    /* Boolean comparison - should become (phi == 1) or (phi == 0) */
    if (condition) {   /* Should trigger uncovered code */
        global_counter += 3;
    }
}

__attribute__((noinline, noipa))
void hot_function_complex(int iterations) {
    /* Combined pattern with multiple phi nodes and comparisons */
    int state = 0;
    int prev_state = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Phi node from loop header */
        int current = (i == 0) ? 0 : prev_state;
        
        /* Nested condition creating another phi */
        int next_state;
        if (current % 3 == 0) {
            next_state = 1;
        } else if (current % 3 == 1) {
            next_state = 0;
        } else {
            next_state = current + 1;
        }
        
        /* Multiple chained assignments */
        int var1 = next_state;
        int var2 = var1;
        int var3 = var2;
        
        /* Two comparisons to increase coverage probability */
        if (var3 == 0) {   /* Comparison with 0 */
            global_counter += i;
        }
        
        if (var3 == 1) {   /* Comparison with 1 */
            result_array[i % 1000] = i;
        }
        
        prev_state = next_state;
    }
}

/* Main driver that ensures hot paths */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Execute all patterns to ensure coverage */
    for (int run = 0; run < 10; ++run) {
        /* Pattern A - loop phi */
        hot_function_phi_loop(iterations / 10, run % 2);
        
        /* Pattern B - merge phi */
        hot_function_merge_phi(run % 2);
        
        /* Pattern C - bool phi */
        hot_function_bool_phi(run, run * 2);
        
        /* Complex pattern */
        hot_function_complex(iterations / 20);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1000; ++i) {
        checksum += result_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
