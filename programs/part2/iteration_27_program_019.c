/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi nodes from loop conditions */
__attribute__((noinline, noipa))
void hot_function_loop_phi(int iterations, int start_val) {
    int x_prev = start_val;
    
    /* Pattern A: Loop-dependent phi node */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back */
        int y = x;      /* GIMPLE_ASSIGN copy */
        int z = y;      /* Another copy */
        
        /* Critical comparison: phi-derived variable vs constant 0 */
        if (z == 0) {   /* This should trigger the uncovered code */
            global_array[i & 0xFF] += 1;
            global_counter++;
        }
        
        /* Another comparison against 1 */
        if (x == 1) {   /* Another phi-derived comparison */
            global_array[(i + 128) & 0xFF] -= 1;
            global_counter--;
        }
        
        x_prev = x;
    }
}

/* Function with merge point phi */
__attribute__((noinline, noipa))
void hot_function_merge_phi(int iterations, int seed) {
    volatile int cond_source = seed; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern B: Merge point phi from conditional assignment */
        int val;
        if (cond_source & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Chain of assignments */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against 1 */
        if (tmp2 == 1) {  /* Should trigger the uncovered code */
            global_counter += 2;
        }
        
        /* Modify condition source to create varying paths */
        cond_source = (cond_source * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* Function with boolean phi */
__attribute__((noinline, noipa))
void hot_function_bool_phi(int iterations) {
    bool flag = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern with boolean phi */
        bool condition;
        if (i % 3 == 0) {
            condition = true;
        } else {
            condition = false;
        }
        
        /* Boolean comparison (compiles to == 1 check) */
        if (condition) {  /* Should become "if (condition == 1)" in GIMPLE */
            global_array[i & 0xFF] = i;
            global_counter++;
        }
        
        /* Inverted check */
        if (!condition) { /* Should become "if (condition == 0)" */
            global_array[(i + 64) & 0xFF] = -i;
            global_counter--;
        }
    }
}

/* Function with nested control flow creating complex phis */
__attribute__((noinline, noipa))
void hot_function_complex_phi(int iterations) {
    int state = 0;
    
    for (int i = 0; i < iterations; ++i) {
        int x;
        
        /* Complex phi with multiple predecessors */
        if (state == 0) {
            x = 0;
            state = 1;
        } else if (state == 1) {
            x = 1;
            state = 2;
        } else {
            x = (global_counter & 1); /* Volatile dependency */
            state = 0;
        }
        
        /* Multiple assignment chains */
        int a = x;
        int b = a;
        int c = b;
        int d = c;
        
        /* Multiple comparisons to increase coverage */
        if (d == 0) {
            global_counter += 3;
        }
        
        if (a == 1) {
            global_counter -= 2;
        }
    }
}

/* Main driver that calls hot functions */
int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default - should be large for profile */
    
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
    
    /* Call all hot functions to create various phi patterns */
    hot_function_loop_phi(iterations / 4, 0);
    hot_function_merge_phi(iterations / 4, 42);
    hot_function_bool_phi(iterations / 4);
    hot_function_complex_phi(iterations / 4);
    
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
