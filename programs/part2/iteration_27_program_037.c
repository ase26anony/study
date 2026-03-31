/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi nodes from loop conditions */
__attribute__((noinline, noipa))
void hot_loop_phi(int iterations) {
    int prev_val = 0;
    int phi_val;
    
    /* Pattern A: Loop-dependent phi node */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: phi_val = (i == 0) ? 0 : prev_val */
        if (i == 0) {
            phi_val = 0;
        } else {
            phi_val = prev_val + (i % 2);
        }
        
        /* Insert chained assignments to test the while loop walking back */
        int tmp1 = phi_val;
        int tmp2 = tmp1;
        int tmp3 = tmp2;
        
        /* Compare against constant 0 - this should trigger the uncovered code */
        if (tmp3 == 0) {
            global_array[i % 256] += 1;
            global_counter++;
        }
        
        /* Another comparison against constant 1 */
        int check_val = (phi_val > 0) ? 1 : 0;
        if (check_val == 1) {
            global_array[(i + 128) % 256] -= 1;
        }
        
        prev_val = phi_val;
    }
}

/* Function to create merge point phi nodes */
__attribute__((noinline, noipa))
void hot_merge_phi(int iterations) {
    volatile int seed = 42; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern B: Merge point phi from conditional assignment */
        int cond_val;
        if ((seed + i) % 3 == 0) {
            cond_val = 1;
        } else {
            cond_val = 0;
        }
        
        /* Pattern C: Chained copies */
        int copy1 = cond_val;
        int copy2 = copy1;
        
        /* Compare against constant 1 */
        if (copy2 == 1) {
            global_counter += 2;
            global_array[i % 256] = i;
        }
        
        /* Another path with comparison against 0 */
        int alt_val = (cond_val == 1) ? 0 : 1;
        if (alt_val == 0) {
            global_counter--;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

/* Function with boolean phi nodes */
__attribute__((noinline, noipa))
void hot_bool_phi(int iterations) {
    bool flag = false;
    bool prev_flag = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi node for boolean */
        bool current_flag;
        if (i % 10 == 0) {
            current_flag = true;
        } else {
            current_flag = prev_flag ^ (i % 3 == 0);
        }
        
        /* Boolean comparison against true (which is 1) */
        if (current_flag == true) {
            global_array[(i * 7) % 256] += 3;
        }
        
        /* Boolean comparison against false (which is 0) */
        if (current_flag == false) {
            global_counter += 5;
        }
        
        prev_flag = current_flag;
        
        /* Mix with integer phi to create more SSA complexity */
        int mixed_val = current_flag ? (i % 5) : (i % 7);
        if (mixed_val == 0) {
            global_array[i % 256] = mixed_val;
        }
    }
}

/* Main function that drives the hot paths */
int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default iterations */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Call hot functions multiple times to ensure profile annotation */
    for (int run = 0; run < 3; run++) {
        hot_loop_phi(iterations / 3);
        hot_merge_phi(iterations / 3);
        hot_bool_phi(iterations / 3);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}
