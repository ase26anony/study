/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi nodes from loop */
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
        
        /* Chain assignments to test the while loop walking back */
        int tmp1 = phi_val;
        int tmp2 = tmp1;
        int tmp3 = tmp2;
        
        /* Compare against constant 0 - this should trigger the uncovered code */
        if (tmp3 == 0) {
            global_array[i % 256] += 1;
            global_counter++;
        }
        
        /* Also compare against constant 1 */
        int tmp4 = phi_val;
        if (tmp4 == 1) {
            global_array[(i + 1) % 256] += 2;
            global_counter += 2;
        }
        
        prev = phi_val;
    }
}

/* Function with merge point phi */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = iterations; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point of conditional */
        int cond_val;
        if (seed & 1) {
            cond_val = 1;
        } else {
            cond_val = 0;
        }
        
        /* Multiple assignments to obscure origin */
        int a = cond_val;
        int b = a;
        
        /* Compare against 1 */
        if (b == 1) {
            global_counter += 3;
            global_array[i % 256] *= 2;
        }
        
        /* Another chain for comparison against 0 */
        int c = cond_val;
        int d = c;
        int e = d;
        if (e == 0) {
            global_counter += 5;
            global_array[(i + 128) % 256] /= 2;
        }
        
        /* Modify seed to create varying patterns */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

/* Complex pattern with nested loops and multiple phi nodes */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int outer_iter, int inner_iter) {
    int outer_phi = 0;
    
    for (int o = 0; o < outer_iter; ++o) {
        /* Outer loop phi */
        int outer_val = (o == 0) ? 1 : outer_phi * 2 % 7;
        
        for (int i = 0; i < inner_iter; ++i) {
            /* Inner loop phi with dependency on outer */
            int inner_phi;
            if (i == 0) {
                inner_phi = outer_val;
            } else {
                inner_phi = (inner_phi + outer_val) % 3;
            }
            
            /* Long chain of assignments */
            int v1 = inner_phi;
            int v2 = v1;
            int v3 = v2;
            int v4 = v3;
            int v5 = v4;
            
            /* Multiple comparisons against 0 and 1 */
            if (v5 == 0) {
                global_counter += 7;
                global_array[(o + i) % 256] |= 0x01;
            }
            
            if (v3 == 1) {
                global_counter += 11;
                global_array[(o * i) % 256] |= 0x02;
            }
        }
        
        outer_phi = outer_val;
    }
}

/* Boolean pattern using _Bool type */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    _Bool flag = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create boolean phi */
        _Bool bphi;
        if (i % 3 == 0) {
            bphi = 1;
        } else {
            bphi = flag;
        }
        
        /* Assignment chain with _Bool */
        _Bool b1 = bphi;
        _Bool b2 = b1;
        
        /* Comparison with boolean constants (which are 0/1) */
        if (b2 == 1) {  /* Should compare against constant 1 */
            global_counter += 13;
            global_array[i % 256] ^= 0xFF;
        }
        
        /* Another comparison */
        _Bool b3 = !bphi;
        if (b3 == 0) {  /* Should compare against constant 0 */
            global_counter += 17;
        }
        
        flag = bphi;
    }
}

/* Mixed types to create different SSA names */
__attribute__((noinline, noipa))
void pattern_e_mixed_types(int iterations) {
    char c_phi = 0;
    short s_phi = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Character phi */
        char c_val;
        if (i % 5 == 0) {
            c_val = 1;
        } else {
            c_val = c_phi + 1;
        }
        
        /* Short phi */
        short s_val;
        if (i % 7 == 0) {
            s_val = 0;
        } else {
            s_val = s_phi - 1;
        }
        
        /* Compare char against 1 */
        char c_tmp = c_val;
        if (c_tmp == 1) {
            global_counter += 19;
        }
        
        /* Compare short against 0 */
        short s_tmp = s_val;
        if (s_tmp == 0) {
            global_counter += 23;
        }
        
        c_phi = c_val;
        s_phi = s_val;
    }
}

/* Main hot function that calls all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    /* Call each pattern to ensure various phi structures are created */
    pattern_a_loop_phi(iterations / 10);
    pattern_b_merge_phi(iterations / 8);
    pattern_c_complex_phi(10, iterations / 100);
    pattern_d_bool_phi(iterations / 12);
    pattern_e_mixed_types(iterations / 15);
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
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function multiple times to ensure hot path */
    for (int run = 0; run < 3; ++run) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
