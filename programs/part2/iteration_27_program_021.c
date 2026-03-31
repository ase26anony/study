/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    int y_prev = 1;
    
    for (int i = 0; i < iterations; ++i) {
        /* x becomes a phi node: φ(x_prev, start_val) */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back */
        int a = x;
        int b = a;
        int c = b;
        
        /* Comparison against 0 - should trigger uncovered code */
        if (c == 0) {
            global_array[i & 255] += 1;
            global_counter++;
        }
        
        /* Another phi pattern with comparison against 1 */
        int y = (i == 0) ? 1 : y_prev * 2;
        int d = y;
        int e = d;
        
        if (e == 1) {
            global_array[(i + 128) & 255] -= 1;
            global_counter--;
        }
        
        x_prev = x;
        y_prev = y;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int flag) {
    /* Pattern B: Merge point phi with comparison against 1 */
    int val;
    
    /* Create a phi node at the merge point */
    if (flag) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Chain assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Comparison against 1 */
    if (tmp2 == 1) {
        global_counter += 100;
    }
    
    /* Another pattern with boolean phi */
    _Bool bool_val;
    if (global_counter > 1000) {
        bool_val = 1;
    } else {
        bool_val = 0;
    }
    
    int tmp3 = bool_val;
    if (tmp3 == 0) {
        global_array[0] = 1;
    }
}

__attribute__((noinline, noipa))
void hot_function_nested(int depth, int *result) {
    /* Pattern C: Complex phi network with multiple comparisons */
    int phi_var;
    
    if (depth > 10) {
        phi_var = 0;
    } else if (depth > 5) {
        phi_var = 1;
    } else {
        phi_var = depth & 1;  /* 0 or 1 */
    }
    
    /* Multiple assignment chain */
    int chain1 = phi_var;
    int chain2 = chain1;
    int chain3 = chain2;
    int chain4 = chain3;
    
    /* Multiple comparisons to increase coverage */
    if (chain4 == 0) {
        *result += 1;
    }
    
    if (chain4 == 1) {
        *result -= 1;
    }
    
    /* Create another phi through arithmetic */
    int another_phi = (phi_var == 0) ? 1 : 0;
    if (another_phi == 1) {
        global_counter++;
    }
}

__attribute__((noinline, noipa))
void hot_function_mixed_types(int iterations) {
    /* Use different types to create distinct SSA names */
    char char_phi = 0;
    short short_phi = 1;
    int int_phi = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Update phi variables */
        char_phi = (i & 1) ? 1 : 0;
        short_phi = (short_phi == 1) ? 0 : 1;
        int_phi = (int_phi + 1) & 1;
        
        /* Chain assignments with type conversions */
        int tmp_char = char_phi;
        int tmp_short = short_phi;
        int tmp_int = int_phi;
        
        /* Multiple comparisons */
        if (tmp_char == 0) {
            global_array[i & 15] = i;
        }
        
        if (tmp_short == 1) {
            global_array[(i + 16) & 15] = i * 2;
        }
        
        if (tmp_int == 0) {
            global_counter += tmp_int;
        }
    }
}

/* Main driver that ensures hot paths are taken */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Execute all patterns to ensure coverage */
    hot_function_phi_loop(iterations / 2, 0);
    hot_function_phi_loop(iterations / 4, 1);
    
    for (int i = 0; i < iterations / 1000; ++i) {
        hot_function_merge_phi(i & 1);
    }
    
    int nested_result = 0;
    for (int i = 0; i < iterations / 100; ++i) {
        hot_function_nested(i % 20, &nested_result);
    }
    
    hot_function_mixed_types(iterations / 10);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
