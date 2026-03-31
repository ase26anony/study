/* test_auto_profile.c - Test program for GCC AutoFDO coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
static int pattern_a_phi_loop(int iterations, int start_val) {
    int x_prev = 0;
    int result = 0;
    
    /* Pattern A: Loop-dependent phi node feeding comparison with 0/1 */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {   /* Should trigger uncovered code when hot */
            result += 1;
            global_array[i & 255] += 1;
        }
        
        /* Another comparison with 1 */
        if (c == 1) {   /* Another chance to trigger */
            result += 2;
            global_counter++;
        }
        
        x_prev = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
static int pattern_b_merge_phi(int cond1, int cond2) {
    int val1, val2;
    
    /* Pattern B: Merge point phi from conditional assignments */
    if (cond1) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (cond2) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* This creates a phi node at the merge point */
    int merged_val = (val1 > val2) ? val1 : val2;
    
    /* Chain assignments */
    int tmp1 = merged_val;
    int tmp2 = tmp1;
    
    /* Comparison with 1 */
    if (tmp2 == 1) {
        return 100;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
static int pattern_c_complex_phi(int mode, int iterations) {
    int result = 0;
    int state = 0;
    
    /* Pattern C: Complex phi network with multiple predecessors */
    for (int i = 0; i < iterations; ++i) {
        int next_state;
        
        /* Multiple conditional updates to create complex phi */
        if (mode == 0) {
            next_state = (state + 1) & 1;  /* Toggle 0/1 */
        } else if (mode == 1) {
            next_state = (state == 0) ? 1 : 0;
        } else {
            next_state = (i & 1);  /* Based on iteration */
        }
        
        /* Chain of assignments */
        int s1 = next_state;
        int s2 = s1;
        int s3 = s2;
        
        /* Multiple comparisons to increase coverage probability */
        if (s3 == 0) {
            result += i;
        }
        if (s3 == 1) {
            result -= i;
        }
        
        state = next_state;
    }
    
    return result;
}

/* Hot function that will be called many times */
__attribute__((noinline, noipa))
static int hot_function(int iterations) {
    int total = 0;
    
    /* Mix all patterns to increase chance of hitting uncovered code */
    total += pattern_a_phi_loop(iterations / 3, rand() & 1);
    total += pattern_b_merge_phi(rand() & 1, rand() & 1);
    total += pattern_c_complex_phi(rand() % 3, iterations / 4);
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - large to ensure hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    /* Seed random for variability but reproducible with same input */
    srand(42);
    
    int total_result = 0;
    
    /* Execute hot function many times to generate profile data */
    for (int i = 0; i < 10; ++i) {
        total_result += hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = total_result;
    for (int i = 0; i < 256; ++i) {
        checksum ^= global_array[i];
    }
    checksum ^= global_counter;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d, Checksum: %d\n", total_result, checksum);
    
    return 0;
}
