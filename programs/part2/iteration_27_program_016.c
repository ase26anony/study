/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Function to create phi node from loop iteration */
__attribute__((noinline, noipa))
void pattern_a_loop_phi(int iterations) {
    int prev_val = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi node: x gets either start value (0) or prev_val + 1 */
        int x;
        if (i == 0) {
            x = 0;  /* Start value */
        } else {
            x = prev_val + 1;  /* Value from previous iteration */
        }
        
        /* Chain assignments to test the while loop walking back */
        int y = x;
        int z = y;
        int w = z;
        
        /* Critical comparison against constant 0 */
        if (w == 0) {
            /* Hot path - executed on first iteration */
            global_array[i % 1024] += 1;
            global_counter++;
        } else {
            /* Cold path - executed on subsequent iterations */
            global_array[i % 1024] -= 1;
        }
        
        prev_val = x;
    }
}

/* Function with merge point phi */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = iterations;  /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi at merge point of conditional */
        int val;
        if ((seed + i) % 3 == 0) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Multiple assignment chain */
        int a = val;
        int b = a;
        
        /* Comparison against constant 1 */
        if (b == 1) {
            /* Hot path - ~33% of iterations */
            global_counter += 2;
            global_array[i % 1024] = i;
        } else {
            /* Cold path */
            global_counter -= 1;
        }
    }
}

/* Complex pattern with nested loops and multiple phis */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int outer_iterations) {
    int state = 0;
    
    for (int outer = 0; outer < outer_iterations; ++outer) {
        /* Outer loop phi */
        int outer_phi = (outer == 0) ? 0 : state;
        
        for (int inner = 0; inner < 10; ++inner) {
            /* Inner loop phi combining outer and inner state */
            int inner_phi;
            if (inner == 0) {
                inner_phi = outer_phi;
            } else {
                inner_phi = (inner_phi + 1) % 2;  /* Self-referential phi */
            }
            
            /* Long chain of assignments */
            int t1 = inner_phi;
            int t2 = t1;
            int t3 = t2;
            int t4 = t3;
            
            /* Multiple comparisons against 0/1 */
            if (t4 == 0) {
                global_array[(outer * 10 + inner) % 1024] += outer;
                global_counter++;
            }
            
            if (t3 == 1) {
                global_array[(outer * 10 + inner) % 1024] -= inner;
                global_counter--;
            }
        }
        
        state = (state + 1) % 2;
    }
}

/* Boolean phi pattern */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    _Bool flag = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Boolean phi */
        _Bool b;
        if (i % 7 == 0) {
            b = 1;  /* true */
        } else {
            b = flag;  /* Previous value */
        }
        
        /* Comparison in boolean context (compiles to == 1) */
        if (b == 1) {
            global_counter += 3;
            global_array[i % 1024] = i * 2;
        }
        
        flag = b;
    }
}

/* Mixed integer types for distinct SSA names */
__attribute__((noinline, noipa))
void pattern_e_mixed_types(int iterations) {
    char c_val = 0;
    short s_val = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Phi with char type */
        char c_phi;
        if (i % 5 == 0) {
            c_phi = 0;
        } else {
            c_phi = c_val + 1;
        }
        
        /* Phi with short type */
        short s_phi;
        if (i % 3 == 0) {
            s_phi = 1;
        } else {
            s_phi = s_val - 1;
        }
        
        /* Chain assignments with type conversions */
        int i1 = c_phi;
        int i2 = s_phi;
        int i3 = i1 + i2;
        
        /* Comparisons */
        if (i1 == 0) {
            global_counter += c_phi;
        }
        
        if (i2 == 1) {
            global_counter -= s_phi;
        }
        
        c_val = c_phi;
        s_val = s_phi;
    }
}

/* Main hot function that calls all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    /* Call each pattern with different weights to create varied profile */
    pattern_a_loop_phi(iterations);          /* ~40% of execution */
    pattern_b_merge_phi(iterations / 2);     /* ~20% of execution */
    pattern_c_complex_phi(iterations / 10);  /* ~10% of execution */
    pattern_d_bool_phi(iterations / 3);      /* ~15% of execution */
    pattern_e_mixed_types(iterations / 4);   /* ~15% of execution */
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
    
    /* Reset globals */
    global_counter = 0;
    memset(global_array, 0, sizeof(global_array));
    
    /* Execute hot function multiple times to ensure profiling */
    for (int run = 0; run < 3; ++run) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
