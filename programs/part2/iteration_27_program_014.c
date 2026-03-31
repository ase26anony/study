/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to maintain SSA structure */
__attribute__((noinline, noipa))
int phi_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison */
    int x_prev = start_val;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* x becomes a phi node: value from start_val (first iteration) 
           or x_prev + 1 (subsequent iterations) */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back */
        int a = x;
        int b = a;
        int c = b;
        
        /* Critical comparison: phi-derived variable vs constant 0 */
        if (c == 0) {
            result += 1;
            global_array[i & 255] += 1;
        }
        
        x_prev = x;
        global_counter++;
    }
    return result;
}

__attribute__((noinline, noipa))
int phi_pattern_b(int iterations) {
    /* Pattern B: Merge point phi from conditional assignment */
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Use volatile to prevent constant folding */
        volatile int cond = (i & 1);
        
        /* val is a phi node at the merge point */
        int val = cond ? 1 : 0;
        
        /* Multiple assignment chain */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against constant 1 */
        if (tmp2 == 1) {
            result += i;
            global_array[(i + 1) & 255] += 1;
        }
        global_counter++;
    }
    return result;
}

__attribute__((noinline, noipa))
int phi_pattern_c(int iterations, int seed) {
    /* Pattern C: Complex phi network with multiple predecessors */
    int result = 0;
    int state = seed;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create multiple paths that merge */
        int phi_var;
        if (state & 1) {
            phi_var = 0;
        } else if (state & 2) {
            phi_var = 1;
        } else {
            phi_var = state & 3;
        }
        
        /* Update state pseudo-randomly */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Long chain of assignments */
        int v1 = phi_var;
        int v2 = v1;
        int v3 = v2;
        int v4 = v3;
        
        /* Both comparisons against 0 and 1 */
        if (v4 == 0) {
            result += 1;
        } else if (v4 == 1) {
            result += 2;
        }
        
        global_counter++;
        global_array[i & 255] = phi_var;
    }
    return result;
}

__attribute__((noinline, noipa))
int hot_function(int iterations) {
    /* Main hot function combining all patterns */
    int total = 0;
    
    /* Execute each pattern multiple times */
    for (int phase = 0; phase < 3; phase++) {
        total += phi_pattern_a(iterations / 3, phase);
        total += phi_pattern_b(iterations / 3);
        total += phi_pattern_c(iterations / 3, phase * 7 + 1);
    }
    
    return total;
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
    
    /* Execute hot function to generate profile */
    int result = hot_function(iterations);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = result;
    for (int i = 0; i < 256; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_counter;
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return 0;
}
