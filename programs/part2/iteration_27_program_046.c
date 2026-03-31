/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch probabilities */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void side_effect(int idx, int val) {
    global_array[idx & 255] += val;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
__attribute__((noinline, noipa))
void pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int phi_val = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi node: phi_val = (i == 0) ? 0 : prev + 1 */
        if (i == 0) {
            phi_val = 0;
        } else {
            phi_val = prev + 1;
        }
        
        /* Chain of assignments to test the while loop walking back */
        int a = phi_val;      /* GIMPLE_ASSIGN from phi */
        int b = a;            /* Another GIMPLE_ASSIGN */
        int c = b;            /* Final GIMPLE_ASSIGN before condition */
        
        /* Critical comparison: c == 0 or c == 1 */
        if (c == 0) {
            /* Hot path - executed frequently */
            side_effect(i, 1);
        } else if (c == 1) {
            /* Also hot - ensure both 0 and 1 comparisons are covered */
            side_effect(i, 2);
        } else {
            /* Cold path - rarely executed */
            if (global_counter > 1000000) {  /* Prevent elimination */
                side_effect(i, 3);
            }
        }
        
        prev = phi_val;
    }
}

/* Pattern B: Merge point phi from conditional */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = 42;  /* Prevent constant folding */
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi at merge point of conditional */
        int cond_val;
        if ((seed + i) & 1) {  /* Volatile prevents optimization */
            cond_val = 1;
        } else {
            cond_val = 0;
        }
        
        /* Direct use of phi result in comparison */
        if (cond_val == 1) {
            /* Hot path */
            side_effect(i, 10);
        } else {
            /* Also hot */
            side_effect(i, 20);
        }
    }
}

/* Pattern C: Complex phi network with multiple predecessors */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int iterations) {
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_var;
        
        /* Create phi with multiple possible incoming values */
        switch (state) {
            case 0:
                phi_var = 0;
                state = 1;
                break;
            case 1:
                phi_var = 1;
                state = 2;
                break;
            case 2:
                phi_var = 0;
                state = 0;
                break;
            default:
                phi_var = 1;
                state = 0;
        }
        
        /* Multiple assignment chain */
        int tmp1 = phi_var;
        int tmp2 = tmp1;
        int final = tmp2;
        
        /* Both 0 and 1 comparisons */
        if (final == 0) {
            side_effect(i, 100);
        }
        if (final == 1) {
            side_effect(i, 200);
        }
    }
}

/* Pattern D: Boolean phi from comparison */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    volatile int threshold = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* Boolean value from comparison creates phi */
        bool is_small = (i < threshold);
        
        /* Chain assignments with different types */
        char c = is_small;    /* Implicit conversion */
        short s = c;
        int final = s;
        
        /* Comparison with 0/1 (bool context) */
        if (final == 1) {     /* true path */
            side_effect(i, 1000);
        } else {              /* false path */
            side_effect(i, 2000);
        }
    }
}

/* Main hot function that combines all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    /* Execute each pattern to create diverse profile */
    pattern_a_loop_phi(iterations / 4);
    pattern_b_merge_phi(iterations / 4);
    pattern_c_complex_phi(iterations / 4);
    pattern_d_bool_phi(iterations / 4);
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - large to ensure hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Call hot function multiple times to ensure profiling */
    for (int run = 0; run < 10; run++) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
