/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {   /* Should trigger the uncovered code */
            global_array[i % 1000] += 1;
            result += 1;
        }
        
        /* Another comparison with 1 */
        if (x == 1) {   /* Another phi comparison with 1 */
            global_counter++;
        }
        
        x_prev = x;
    }
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        printf("Unexpected large result\n");
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int flag1, int flag2) {
    /* Pattern B: Merge point phi from conditional assignment */
    int val1, val2;
    
    /* Create two independent conditions */
    if (flag1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (flag2 > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Merge point phi: val = phi(val1, val2) */
    int val = (global_counter % 2) ? val1 : val2;
    
    /* Chain assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Comparison with 1 */
    if (tmp2 == 1) {   /* Should trigger the uncovered code */
        global_array[global_counter % 1000] += 2;
    }
    
    /* Another comparison with 0 */
    if (val == 0) {    /* Direct phi comparison with 0 */
        global_counter += 2;
    }
}

__attribute__((noinline, noipa))
void hot_function_bool_phi(int a, int b) {
    /* Pattern C: Boolean phi node */
    bool cond1 = (a > 0);
    bool cond2 = (b > 0);
    
    /* Phi node for boolean value */
    bool final_cond = (global_counter % 3) ? cond1 : cond2;
    
    /* Boolean comparison (compiles to == 1 or == 0) */
    if (final_cond) {   /* Should become if (final_cond == 1) */
        global_array[global_counter % 1000] += 3;
    }
    
    /* Explicit comparison with 0 */
    if (final_cond == 0) {
        global_counter += 3;
    }
}

__attribute__((noinline, noipa))
void hot_function_complex_chains(int n) {
    /* Pattern D: Complex chain of assignments with multiple phis */
    int x = 0;
    int y = 1;
    
    for (int i = 0; i < n; ++i) {
        /* Two independent phis in the loop */
        int phi1 = (i == 0) ? 0 : x + 1;
        int phi2 = (i == 0) ? 1 : y * 2;
        
        /* Complex chain of assignments */
        int a = phi1;
        int b = phi2;
        int c = a + b;
        int d = c;
        int e = d;
        
        /* Multiple comparisons with 0 and 1 */
        if (e == 0) {
            global_array[i % 1000] += 4;
        }
        
        if (phi1 == 1) {
            global_counter += 4;
        }
        
        if (phi2 == 0) {
            global_array[(i + 1) % 1000] += 5;
        }
        
        x = phi1;
        y = phi2 % 10;  /* Keep values bounded */
    }
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
    
    /* Execute hot functions multiple times to ensure profile annotation */
    for (int i = 0; i < 10; ++i) {
        /* Mix different patterns to cover various phi scenarios */
        hot_function_phi_loop(iterations / 10, i % 3);
        hot_function_merge_phi(i, i * 2);
        hot_function_bool_phi(i, i + 1);
        hot_function_complex_chains(iterations / 100);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1000; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
