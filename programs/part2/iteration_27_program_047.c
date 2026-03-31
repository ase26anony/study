/* test_auto_profile.c - AutoFDO coverage test for phi-node conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi with chained copies */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable vs constant 0 */
        if (c == 0) {
            /* Hot path - executed many times */
            global_array[i & 255] += 1;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            global_array[i & 255] -= 1;
        }
        
        /* Another comparison with constant 1 */
        if (x == 1) {
            global_counter += 2;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_b(int iterations) {
    /* Pattern B: Merge point phi from conditional assignment */
    volatile int seed = iterations; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point of conditional */
        int cond_val = (seed & 1) ? 1 : 0;  /* phi node here */
        
        /* Chain assignments */
        int tmp1 = cond_val;
        int tmp2 = tmp1;
        
        /* Comparison with constant 1 */
        if (tmp2 == 1) {
            /* Hot path */
            global_counter += 3;
            global_array[(i * 17) & 255] ^= tmp2;
        }
        
        /* Flip seed to create varying conditions */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_c(int iterations, int mode) {
    /* Pattern C: Complex phi network with multiple predecessors */
    int state = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Multiple potential phi sources */
        int val;
        if (mode == 0) {
            val = i & 1;            /* Source 1 */
        } else if (mode == 1) {
            val = (i >> 1) & 1;     /* Source 2 */
        } else {
            val = state;            /* Source 3 - phi from previous iteration */
        }
        
        /* Update state for next iteration (creates loop-carried phi) */
        state = val ^ (state << 1);
        
        /* Multiple assignment chain */
        int v1 = val;
        int v2 = v1;
        int v3 = v2;
        
        /* Comparisons with both 0 and 1 */
        if (v3 == 0) {
            global_counter += 5;
            global_array[i & 255] |= 0x01;
        }
        
        if (val == 1) {
            global_counter += 7;
            global_array[i & 255] |= 0x02;
        }
    }
}

/* Helper to create side effects and prevent dead code elimination */
__attribute__((noinline, noipa))
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 256; ++i) {
        sum = (sum * 31 + global_array[i]) & 0xffff;
    }
    return sum + global_counter;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - large enough to be hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Reset globals */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute all patterns to create diverse profile data */
    
    /* Pattern A with start_val = 0 to trigger c == 0 path */
    hot_function_pattern_a(iterations / 3, 0);
    
    /* Pattern A with start_val = 1 to trigger different paths */
    hot_function_pattern_a(iterations / 3, 1);
    
    /* Pattern B - creates merge point phi */
    hot_function_pattern_b(iterations / 3);
    
    /* Pattern C - complex phi network */
    for (int mode = 0; mode < 3; mode++) {
        hot_function_pattern_c(iterations / 9, mode);
    }
    
    /* Compute and print checksum to ensure code isn't eliminated */
    int checksum = compute_checksum();
    printf("Checksum: %d (Iterations: %d)\n", checksum, iterations);
    
    return 0;
}
