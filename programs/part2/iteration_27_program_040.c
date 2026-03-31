/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Function to create phi node from loop header */
__attribute__((noinline, noipa))
int create_loop_phi(int iterations, int start_val) {
    int phi_val = start_val;
    int prev_val = 0;
    
    /* Pattern A: Loop-dependent phi node */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: phi_val = (i == 0) ? start_val : prev_val + 1 */
        if (i == 0) {
            phi_val = start_val;
        } else {
            phi_val = prev_val + 1;
        }
        
        /* Chain of assignments to test the while loop walking back through GIMPLE_ASSIGN */
        /* Pattern C: Chained copies */
        int copy1 = phi_val;
        int copy2 = copy1;
        int final_val = copy2;
        
        /* Critical comparison against constant 0 */
        if (final_val == 0) {
            /* Hot path - executed frequently */
            global_array[i % 1000] += 1;
            global_counter++;
        } else if (final_val == 1) {
            /* Also compare against 1 */
            global_array[i % 1000] += 2;
            global_counter += 2;
        }
        
        prev_val = phi_val;
    }
    
    return phi_val;
}

/* Function with merge point phi */
__attribute__((noinline, noipa))
int create_merge_phi(int a, int b) {
    int result;
    
    /* Pattern B: Merge point phi from conditional */
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    /* Insert assignment chain */
    int tmp1 = result;
    int tmp2 = tmp1;
    int final = tmp2;
    
    /* Compare phi result against 1 */
    if (final == 1) {
        global_counter += 10;
        return 1;
    }
    
    return 0;
}

/* Complex function mixing patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    int start_val = 0;
    
    /* Mix of patterns to create various phi nodes */
    for (int outer = 0; outer < 10; ++outer) {
        /* Pattern A with varying start values */
        int loop_result = create_loop_phi(iterations / 10, start_val);
        
        /* Pattern B with loop result */
        int merge_result = create_merge_phi(loop_result, outer);
        
        /* Another phi from conditional inside loop */
        int inner_phi;
        for (int i = 0; i < 100; ++i) {
            /* Create phi that depends on loop iteration */
            if (i % 2 == 0) {
                inner_phi = 0;
            } else {
                inner_phi = 1;
            }
            
            /* Chain assignments */
            int chain1 = inner_phi;
            int chain2 = chain1;
            int chain3 = chain2;
            
            /* Compare against both 0 and 1 */
            if (chain3 == 0) {
                global_array[(i + outer) % 1000] += 3;
            }
            if (chain3 == 1) {
                global_array[(i + outer) % 1000] += 5;
            }
        }
        
        /* Alternate start_val to create different phi values */
        start_val = (start_val == 0) ? 1 : 0;
    }
}

/* Helper to prevent dead code elimination */
__attribute__((noinline, noipa))
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += global_array[i];
    }
    return sum + global_counter;
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
    
    /* Execute hot function many times to generate profile */
    hot_function(iterations);
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum();
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
