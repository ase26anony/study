/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi nodes from loop conditions */
__attribute__((noinline, noipa))
void hot_loop_phi(int iterations) {
    int prev = 0;
    
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? 1 : prev + 1 */
        int x = (i == 0) ? 1 : prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        /* Pattern C: Chained copies */
        int a = x;
        int b = a;
        int c = b;
        
        /* Comparison against constant 0 */
        if (c == 0) {
            /* Cold path - rarely taken */
            global_array[0]++;
        } else {
            /* Hot path - always taken (since x starts at 1 and increments) */
            global_array[c % 256]++;
            global_counter++;
        }
        
        prev = x;
    }
}

/* Function to create phi nodes from conditional merges */
__attribute__((noinline, noipa))
void hot_cond_phi(int iterations) {
    volatile int seed = 42; /* volatile to prevent constant folding */
    
    /* Pattern B: Merge point phi */
    for (int i = 0; i < iterations; ++i) {
        /* Create a condition that's usually true but not always predictable */
        bool cond = (seed % 100) < 95; /* 95% true */
        seed = seed * 1103515245 + 12345; /* Simple PRNG */
        
        /* This creates a phi node at the merge point */
        int val = cond ? 1 : 0;
        
        /* Multiple assignment chains */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against constant 1 */
        if (tmp2 == 1) {
            /* Hot path - 95% of iterations */
            global_counter += 2;
            global_array[(i * 17) % 256]++;
        } else {
            /* Cold path - 5% of iterations */
            global_array[255]++;
        }
    }
}

/* Mixed type phi nodes to create distinct SSA names */
__attribute__((noinline, noipa))
void hot_mixed_types(int iterations) {
    char char_prev = 0;
    short short_prev = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Character type phi */
        char c = (i == 0) ? 1 : char_prev + 1;
        char c_copy = c;
        
        /* Short type phi */
        short s = (i < 10) ? 0 : 1;
        short s_copy = s;
        
        /* Comparisons with different types */
        if (c_copy == 0) {
            global_array[1]++;
        }
        
        if (s_copy == 1) {
            global_counter += 3;
            global_array[(i * 13) % 256]++;
        }
        
        char_prev = c;
        short_prev = s;
    }
}

/* Complex nested phi structure */
__attribute__((noinline, noipa))
void hot_nested_phi(int iterations) {
    int state = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Outer phi */
        int outer = (state == 0) ? 1 : 0;
        
        /* Inner phi based on outer */
        int inner;
        if (outer == 1) {
            inner = (i % 2 == 0) ? 1 : 0;
        } else {
            inner = 0;
        }
        
        /* Chain assignments */
        int v1 = inner;
        int v2 = v1;
        int v3 = v2;
        
        /* Multiple comparisons to increase coverage */
        if (v3 == 0) {
            global_array[2]++;
            state = 1;
        } else {
            global_counter += 4;
            global_array[(i * 19) % 256]++;
            state = 0;
        }
    }
}

/* Main driver function */
int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default iterations */
    
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
    
    /* Execute all hot functions to generate diverse profile data */
    hot_loop_phi(iterations);
    hot_cond_phi(iterations);
    hot_mixed_types(iterations);
    hot_nested_phi(iterations);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Sample array output */
    printf("Array samples: [0]=%d, [1]=%d, [2]=%d, [100]=%d\n",
           global_array[0], global_array[1], global_array[2], global_array[100]);
    
    return 0;
}
