/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 - this should trigger the uncovered code */
        if (c == 0) {
            /* Hot path - executed many times */
            global_array[i % 1024] += 1;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            global_array[i % 1024] -= 1;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int iterations, int flag) {
    /* Pattern B: Merge point phi feeding comparison with 1 */
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point of conditional */
        int val;
        if (flag) {
            val = 1;  /* One arm of phi */
        } else {
            val = 0;  /* Other arm of phi */
        }
        
        /* Insert chained copies */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against constant 1 */
        if (tmp2 == 1) {
            /* Hot path */
            global_counter += 2;
            global_array[(i * 7) % 1024] ^= val;
        }
    }
}

__attribute__((noinline, noipa))
void hot_function_bool_phi(int iterations, _Bool initial) {
    /* Pattern C: Boolean phi node (comparison with 0/1 implicit) */
    _Bool state = initial;
    
    for (int i = 0; i < iterations; ++i) {
        /* Toggle state - creates phi node */
        state = !state;
        
        /* Chain through different types to create distinct SSA names */
        char c = state;
        short s = c;
        int n = s;
        
        /* Comparison with boolean constant (which is 0 or 1) */
        if (n == 1) {  /* Equivalent to if (state == true) */
            global_counter += 3;
            global_array[i % 1024] |= 0xFF;
        }
        
        /* Another comparison with 0 */
        if (n == 0) {
            global_array[i % 1024] &= 0x0F;
        }
    }
}

__attribute__((noinline, noipa))
void hot_function_complex_phi(int iterations) {
    /* More complex pattern with nested loops and multiple phis */
    int outer_phi = 0;
    
    for (int j = 0; j < 10; ++j) {
        /* Phi from loop header */
        int inner_phi = outer_phi;
        
        for (int i = 0; i < iterations/10; ++i) {
            /* Phi from previous iteration */
            inner_phi = (inner_phi + i) % 2;
            
            /* Multiple chained assignments */
            int v1 = inner_phi;
            int v2 = v1;
            int v3 = v2;
            
            /* Multiple comparisons with 0/1 */
            if (v3 == 0) {
                global_counter += 4;
            }
            
            if (v3 == 1) {
                global_counter -= 1;
            }
        }
        
        outer_phi = (outer_phi + 1) % 2;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - large enough to create hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Use volatile to prevent compile-time evaluation */
    volatile int start_val = 0;
    volatile int flag = 1;
    volatile _Bool initial = 0;
    
    /* Call all hot functions to create various phi patterns */
    hot_function_phi_loop(iterations, start_val);
    hot_function_merge_phi(iterations / 2, flag);
    hot_function_bool_phi(iterations / 3, initial);
    hot_function_complex_phi(iterations / 4);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
