/* test_auto_profile.c - Test program for AutoFDO phi node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int hot_function_phi_loop(int iterations, int start_val) {
    int result = 0;
    int prev_x = start_val;
    
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : prev_x + 1 */
        int x = (i == 0) ? start_val : prev_x + 1;
        
        /* Chain of assignments to test the while loop walking back */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable vs constant 0 */
        if (c == 0) {   /* Should trigger uncovered lines */
            result += 1;
            global_array[i % 1024] = i;
        }
        
        /* Another comparison with constant 1 */
        int d = (x > 5) ? 1 : 0;  /* Creates another phi node */
        int e = d;                 /* Single assignment copy */
        if (e == 1) {              /* Comparison with 1 */
            result += 2;
            global_counter++;
        }
        
        prev_x = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_merge_phi(int flag1, int flag2) {
    int result = 0;
    
    /* Pattern B: Merge point phi from conditional assignment */
    int val1, val2;
    
    if (flag1) {
        val1 = 1;
        val2 = 0;
    } else {
        val1 = 0;
        val2 = 1;
    }
    
    /* val1 and val2 are phi nodes at the merge point */
    
    /* Chain assignments to obscure origin */
    int tmp1 = val1;
    int tmp2 = tmp1;
    
    /* Comparison with constant 0 */
    if (tmp2 == 0) {
        result += 10;
    }
    
    /* Direct phi use with comparison to 1 */
    if (val2 == 1) {
        result += 20;
    }
    
    /* Boolean phi node */
    bool bval = (flag1 && flag2);
    bool bcopy = bval;
    if (bcopy == true) {  /* Should become comparison with 1 */
        result += 30;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_complex_phi(int mode) {
    int result = 0;
    static int state = 0;
    
    /* Pattern C: Complex phi network with multiple predecessors */
    int phi_var;
    
    switch (mode % 4) {
        case 0:
            phi_var = 0;
            break;
        case 1:
            phi_var = 1;
            break;
        case 2:
            phi_var = state;
            break;
        default:
            phi_var = 2;
    }
    
    /* Multiple assignment chain */
    int chain1 = phi_var;
    int chain2 = chain1;
    int chain3 = chain2;
    
    /* Comparisons that should trigger the uncovered code */
    if (chain3 == 0) {
        result += 100;
    }
    
    if (chain3 == 1) {
        result += 200;
    }
    
    state = (state + 1) % 3;
    return result;
}

/* Main driver that ensures hot paths are taken */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    int total_result = 0;
    
    /* Execute hot functions many times to ensure profile annotation */
    for (int i = 0; i < 10; ++i) {
        /* Pattern A - loop with phi */
        total_result += hot_function_phi_loop(iterations / 10, i % 3);
        
        /* Pattern B - merge phi */
        total_result += hot_function_merge_phi(i & 1, (i >> 1) & 1);
        
        /* Pattern C - complex phi */
        total_result += hot_function_complex_phi(i);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    printf("Global counter: %d\n", global_counter);
    
    /* Simple checksum of global array */
    int array_sum = 0;
    for (int i = 0; i < 1024; ++i) {
        array_sum += global_array[i];
    }
    printf("Array sum: %d\n", array_sum);
    
    return total_result != 0 ? 0 : 1;
}
