/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

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
    int prev_x = 0;
    int x = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi node: x = (i == 0) ? 0 : prev_x + 1 */
        if (i == 0) {
            x = 0;
        } else {
            x = prev_x + 1;
        }
        
        /* Chain of assignments to test the while loop in auto-profile.cc */
        int a = x;      /* GIMPLE_ASSIGN copy */
        int b = a;      /* Another copy */
        int c = b;      /* Final copy before comparison */
        
        /* Comparison against constant 0 */
        if (c == 0) {   /* This should trigger the uncovered code */
            side_effect(i, 1);
        } else {
            side_effect(i, -1);
        }
        
        /* Also test comparison against constant 1 */
        int d = x;
        if (d == 1) {   /* Another test case */
            side_effect(i + 128, 2);
        }
        
        prev_x = x;
    }
}

/* Pattern B: Conditional merge phi */
__attribute__((noinline, noipa))
int pattern_b_merge_phi(int input, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi at merge point */
        int val;
        if (input > i) {
            val = 1;  /* One arm of phi */
        } else {
            val = 0;  /* Other arm of phi */
        }
        
        /* Multiple assignments to obscure origin */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against 1 */
        if (tmp2 == 1) {  /* Should trigger uncovered code */
            result += i;
            side_effect(i, 3);
        }
        
        /* Modify input to create varying behavior */
        input = (input * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Pattern C: Complex phi network with boolean */
__attribute__((noinline, noipa))
void pattern_c_bool_phi(int iterations) {
    bool flag1 = false;
    bool flag2 = true;
    
    for (int i = 0; i < iterations; i++) {
        /* Create multiple phi nodes */
        bool cond;
        if (i % 3 == 0) {
            cond = flag1;
        } else if (i % 3 == 1) {
            cond = flag2;
        } else {
            cond = (i % 2) == 0;
        }
        
        /* Boolean comparison (compiles to == 1 or == 0) */
        if (cond) {  /* Implicit comparison with true/1 */
            side_effect(i, 5);
        }
        
        /* Explicit comparison with 0 */
        bool negated = !cond;
        if (negated == 0) {  /* Should trigger uncovered code */
            side_effect(i + 64, 7);
        }
        
        /* Update flags to prevent constant folding */
        flag1 = (i % 5) == 0;
        flag2 = (i % 7) == 0;
    }
}

/* Pattern D: Nested loops with phi propagation */
__attribute__((noinline, noipa))
int pattern_d_nested_phi(int outer_iter, int inner_iter) {
    int total = 0;
    int outer_acc = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner_phi = outer_acc;  /* Phi from outer loop */
        
        for (int i = 0; i < inner_iter; i++) {
            /* Phi that depends on previous iteration */
            int val;
            if (i == 0) {
                val = inner_phi;
            } else {
                val = total + 1;
            }
            
            /* Chain assignments */
            int v1 = val;
            int v2 = v1;
            
            /* Comparison against 0 */
            if (v2 == 0) {  /* Should trigger uncovered code */
                total += o;
                side_effect(o * inner_iter + i, 11);
            }
            
            total = (total * 1664525 + 1013904223) & 0x7fffffff;
        }
        
        outer_acc = (outer_acc + 1) & 0xf;
    }
    
    return total;
}

/* Main hot function that exercises all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    /* Mix patterns to create varied control flow */
    pattern_a_loop_phi(iterations / 4);
    
    int seed = iterations;
    int result_b = pattern_b_merge_phi(seed, iterations / 8);
    
    pattern_c_bool_phi(iterations / 4);
    
    int result_d = pattern_d_nested_phi(10, iterations / 40);
    
    /* Use results to prevent dead code elimination */
    global_array[0] += result_b + result_d;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for iteration count, default large */
    int iterations = 1000000;
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
    for (int run = 0; run < 3; run++) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum = (checksum * 31 + global_array[i]) & 0x7fffffff;
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
