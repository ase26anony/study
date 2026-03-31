/* test_auto_profile.c - Test program for GCC AutoFDO coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int hot_function(int iteration, int start_val) {
    /* Pattern A: Loop-dependent phi node with chained copies */
    int phi_val;
    
    if (iteration == 0) {
        phi_val = start_val;  /* First iteration value */
    } else {
        phi_val = global_array[iteration % 1000] + 1;  /* Phi from previous iteration */
    }
    
    /* Chain of assignments to test the while loop walking back */
    int copy1 = phi_val;
    int copy2 = copy1;
    int copy3 = copy2;
    
    /* Comparison against constant 0 - this should trigger the uncovered code */
    if (copy3 == 0) {
        /* Hot path - executed many times */
        global_counter += 1;
        return 1;
    }
    
    /* Comparison against constant 1 in another context */
    int bool_val = (phi_val > 100) ? 1 : 0;
    if (bool_val == 1) {
        global_counter += 2;
        return 2;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int merge_point_function(int a, int b) {
    /* Pattern B: Merge point phi with boolean comparison */
    int merged_val;
    
    /* Create a phi at merge point */
    if (a > b) {
        merged_val = 1;
    } else {
        merged_val = 0;
    }
    
    /* Chain assignments */
    int temp1 = merged_val;
    int temp2 = temp1;
    
    /* Compare against 1 */
    if (temp2 == 1) {
        global_counter += 3;
        return a;
    }
    
    return b;
}

__attribute__((noinline, noipa))
void complex_loop(int iterations) {
    /* Pattern C: Complex loop with multiple phi nodes */
    int prev1 = 0, prev2 = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi nodes for loop-carried dependencies */
        int current1, current2;
        
        if (i == 0) {
            current1 = 0;
            current2 = 1;
        } else {
            current1 = prev1 + prev2;
            current2 = prev1 - prev2;
        }
        
        /* Multiple chained copies */
        int chain1 = current1;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Compare against 0 */
        if (chain3 == 0) {
            global_array[i % 1000] += 1;
        }
        
        /* Another comparison against 1 */
        int check = (current2 % 2 == 0) ? 1 : 0;
        if (check == 1) {
            global_array[(i + 1) % 1000] += 2;
        }
        
        prev1 = current1;
        prev2 = current2;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running %d iterations...\n", iterations);
    
    /* Execute hot paths to generate profile data */
    for (int i = 0; i < iterations; i++) {
        /* Call hot_function with varying inputs to create different paths */
        int result1 = hot_function(i, i % 10);
        
        /* Call merge_point_function occasionally */
        if (i % 100 == 0) {
            int result2 = merge_point_function(i, i + 1);
            (void)result2;  /* Use result to prevent dead code elimination */
        }
        
        /* Call complex_loop occasionally with smaller iterations */
        if (i % 1000 == 0) {
            complex_loop(100);
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
