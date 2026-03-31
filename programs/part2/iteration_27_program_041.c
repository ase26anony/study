/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
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
        int a = x;      /* GIMPLE_ASSIGN copy */
        int b = a;      /* Another copy */
        int c = b;      /* Final copy before comparison */
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {   /* This should trigger the uncovered code */
            result += 1;
            global_array[i & 255] = i;
        }
        
        prev_x = x;
        global_counter++;  /* Side effect to prevent dead code elimination */
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_merge_phi(int flag, int val1, int val2) {
    /* Pattern B: Merge point phi feeding comparison with 1 */
    int merged_val;
    
    /* This creates a phi node at the merge point */
    if (flag) {
        merged_val = val1;
    } else {
        merged_val = val2;
    }
    
    /* Chain of assignments */
    int tmp1 = merged_val;
    int tmp2 = tmp1;
    
    /* Comparison with 1 */
    if (tmp2 == 1) {   /* Should trigger uncovered code */
        global_array[flag & 255] += 1;
        return 1;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int hot_function_bool_phi(int a, int b) {
    /* Pattern using boolean phi */
    _Bool cond1 = (a > 0);
    _Bool cond2 = (b > 0);
    
    /* Phi node for boolean value */
    _Bool final_cond = cond1 && cond2;
    
    /* Assignment chain */
    _Bool b1 = final_cond;
    _Bool b2 = b1;
    
    /* Comparison with 1 (true) */
    if (b2 == 1) {   /* Should trigger uncovered code */
        return a + b;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
void complex_phi_pattern(int n) {
    /* More complex pattern with nested loops and multiple phis */
    int x = 0;
    int y = 1;
    
    for (int i = 0; i < n; i++) {
        /* Phi for loop induction */
        int loop_phi = (i == 0) ? 0 : x;
        
        for (int j = 0; j < 10; j++) {
            /* Nested phi */
            int nested_phi = (j == 0) ? loop_phi : y;
            
            /* Assignment chain */
            int chain1 = nested_phi;
            int chain2 = chain1;
            int chain3 = chain2;
            
            /* Multiple comparisons with 0 and 1 */
            if (chain3 == 0) {
                global_counter += i;
            }
            
            if (chain3 == 1) {
                global_counter += j;
            }
            
            y = nested_phi + 1;
        }
        
        x = loop_phi + 1;
        global_array[i & 255] = x;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    int total = 0;
    
    /* Execute hot functions to generate profile data */
    for (int run = 0; run < 10; run++) {
        /* Mix different patterns to cover various cases */
        total += hot_function_phi_loop(iterations / 10, run % 3);
        total += hot_function_merge_phi(run & 1, 1, 0);
        total += hot_function_bool_phi(run, iterations);
        
        if (run % 3 == 0) {
            complex_phi_pattern(iterations / 100);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    checksum += total;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
