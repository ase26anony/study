/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi nodes from loop conditions - Pattern A */
__attribute__((noinline, noipa))
void pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int phi_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: phi_val = (i == 0) ? 0 : prev + 1 */
        if (i == 0) {
            phi_val = 0;
        } else {
            phi_val = prev + 1;
        }
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN - Pattern C */
        int tmp1 = phi_val;
        int tmp2 = tmp1;
        int final_val = tmp2;
        
        /* Compare against constant 0 - this should trigger the uncovered code */
        if (final_val == 0) {
            global_array[i % 256] += 1;
            global_counter++;
        }
        
        /* Compare against constant 1 */
        if (final_val == 1) {
            global_array[(i + 1) % 256] += 2;
            global_counter += 2;
        }
        
        prev = phi_val;
    }
}

/* Function with merge point phi - Pattern B */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations, int seed) {
    volatile int cond_source = seed;  /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi at merge point */
        int phi_val;
        if (cond_source & 1) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Chain through different types to create distinct SSA names */
        char c = (char)phi_val;
        short s = (short)c;
        int final_val = (int)s;
        
        /* Multiple comparisons against 0 and 1 */
        if (final_val == 0) {
            global_counter += 3;
            global_array[i % 256] ^= 0xAA;
        }
        
        if (final_val == 1) {
            global_counter += 5;
            global_array[(i + 128) % 256] ^= 0x55;
        }
        
        /* Change condition source to create varying paths */
        cond_source = (cond_source * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* Complex nested loop with multiple phi nodes */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int outer_iter, int inner_iter) {
    int outer_prev = 0;
    
    for (int o = 0; o < outer_iter; ++o) {
        int inner_phi;
        
        /* Outer loop phi */
        if (o == 0) {
            inner_phi = 1;
        } else {
            inner_phi = outer_prev ^ 1;  /* Toggle between 0 and 1 */
        }
        
        for (int i = 0; i < inner_iter; ++i) {
            /* Inner loop phi based on outer phi and iteration */
            int val;
            if (i == 0) {
                val = inner_phi;
            } else {
                val = (val + i) & 1;  /* Keep it as 0 or 1 */
            }
            
            /* Multiple assignment chain */
            int a = val;
            int b = a;
            int c = b;
            
            /* Both comparisons to ensure coverage */
            if (c == 0) {
                global_counter += 7;
                global_array[(o * i) % 256] |= 0x01;
            }
            
            if (c == 1) {
                global_counter += 11;
                global_array[(o * i + 1) % 256] |= 0x02;
            }
        }
        
        outer_prev = inner_phi;
    }
}

/* Boolean phi pattern */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    bool flag = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Boolean phi that toggles */
        bool b;
        if (i % 3 == 0) {
            b = true;
        } else {
            b = flag;
        }
        
        /* Boolean comparisons compile to == 0 or == 1 */
        if (b == true) {  /* Should become == 1 */
            global_counter += 13;
            global_array[i % 256] *= 3;
        }
        
        if (!b) {  /* Should become == 0 */
            global_counter += 17;
            global_array[(i + 64) % 256] /= 2;
        }
        
        flag = !flag;
    }
}

/* Main hot function that calls all patterns */
__attribute__((noinline, noipa))
void hot_function(int total_iterations) {
    /* Call each pattern with different hot/cold ratios */
    int iter1 = total_iterations / 4;
    int iter2 = total_iterations / 4;
    int iter3 = total_iterations / 4;
    int iter4 = total_iterations / 4;
    
    /* Make pattern A and B hotter */
    iter1 = total_iterations * 3 / 8;
    iter2 = total_iterations * 3 / 8;
    iter3 = total_iterations / 8;
    iter4 = total_iterations / 8;
    
    pattern_a_loop_phi(iter1);
    pattern_b_merge_phi(iter2, 42);
    pattern_c_complex_phi(100, iter3 / 100 + 1);
    pattern_d_bool_phi(iter4);
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
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function multiple times to ensure profiling */
    for (int run = 0; run < 3; ++run) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum ^= global_array[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;  /* Return non-zero if everything was optimized away */
}
