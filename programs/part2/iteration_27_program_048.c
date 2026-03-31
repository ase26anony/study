/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
static int phi_defined_variable(int iteration, int prev_val) {
    /* Pattern B: Merge point phi */
    int phi_val;
    if (iteration & 1) {
        phi_val = 1;  /* Hot path - executed most often */
    } else {
        phi_val = 0;  /* Cold path */
    }
    
    /* Pattern C: Chained copies to test the while loop walking back */
    int copy1 = phi_val;
    int copy2 = copy1;
    int copy3 = copy2;
    
    /* Critical comparison against constant 1 */
    if (copy3 == 1) {  /* This should trigger the uncovered code */
        global_array[iteration & 0xFF] += 1;
        return 1;
    } else {
        global_array[iteration & 0xFF] -= 1;
        return 0;
    }
}

__attribute__((noinline, noipa))
static void hot_function_loop(int iterations) {
    int prev = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern A: Loop-dependent phi with chained copies */
        int loop_phi;
        if (i == 0) {
            loop_phi = 0;
        } else {
            /* This creates a phi node merging previous value with new value */
            loop_phi = prev + (i & 1);
        }
        
        /* More chained copies */
        int tmp1 = loop_phi;
        int tmp2 = tmp1;
        int final_val = tmp2;
        
        /* Comparison against constant 0 */
        if (final_val == 0) {  /* Another trigger for uncovered code */
            global_counter += i;
        }
        
        /* Call function with merge point phi */
        int result = phi_defined_variable(i, prev);
        prev = result;
        
        /* Use volatile to prevent dead code elimination */
        asm volatile("" : "+r" (global_counter));
    }
}

__attribute__((noinline, noipa))
static void complex_phi_pattern(int seed, int iterations) {
    /* More complex pattern with nested loops and multiple phis */
    int state = seed;
    
    for (int i = 0; i < iterations; i++) {
        int inner_phi;
        
        /* Create phi from loop header */
        if (i == 0) {
            inner_phi = state;
        } else {
            inner_phi = (state + i) & 1;
        }
        
        /* Multiple assignment chain */
        int chain1 = inner_phi;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3;
        
        /* Boolean comparison (should become == 1) */
        _Bool bool_val = chain4 > 0;
        if (bool_val == 1) {  /* Boolean comparison with 1 */
            global_array[i & 0xFF] ^= i;
        }
        
        /* Update state with external call to prevent constant folding */
        state = rand() & 1;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - ensures hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Execute hot functions to generate profile data */
    hot_function_loop(iterations);
    complex_phi_pattern(1, iterations / 2);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
