/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Function to create side effects and prevent dead code elimination */
__attribute__((noinline)) void side_effect(int value) {
    global_array[value % 1000] ^= value;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
__attribute__((noinline, noipa)) 
void pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int temp1, temp2, temp3;
    
    for (int i = 0; i < iterations; i++) {
        /* Create a phi node: x is phi(0, x_prev + 1) */
        int x = (i == 0) ? 0 : prev + 1;
        
        /* Chain of assignments to test the while loop walking back */
        temp1 = x;      /* GIMPLE_ASSIGN 1 */
        temp2 = temp1;  /* GIMPLE_ASSIGN 2 */
        temp3 = temp2;  /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 (phi -> temp1 -> temp2 -> temp3) */
        if (temp3 == 0) {
            side_effect(i);
        }
        
        /* Another comparison against constant 1 */
        if (x == 1) {
            side_effect(i * 2);
        }
        
        prev = x;
    }
}

/* Pattern B: Merge point phi from conditional assignment */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations, int seed) {
    volatile int cond_source = seed; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi at merge point of conditional */
        int val;
        if (cond_source & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Direct comparison of phi result against 1 */
        if (val == 1) {
            side_effect(i + 1000);
        }
        
        /* Chain assignments and compare against 0 */
        int chain = val;
        int chain2 = chain;
        if (chain2 == 0) {
            side_effect(i + 2000);
        }
        
        /* Modify condition source to create varying paths */
        cond_source = (cond_source * 1103515245 + 12345) & 0x7fffffff;
    }
}

/* Pattern C: Nested loop with complex phi structure */
__attribute__((noinline, noipa))
void pattern_c_nested_phi(int outer_iter, int inner_iter) {
    int outer_state = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner_prev = 0;
        
        for (int i = 0; i < inner_iter; i++) {
            /* Complex phi: depends on outer loop and previous inner iteration */
            int complex_val;
            if (i == 0) {
                complex_val = outer_state;
            } else {
                complex_val = inner_prev + outer_state;
            }
            
            /* Multiple chained assignments */
            int a = complex_val;
            int b = a;
            int c = b;
            int d = c;
            
            /* Compare chained phi against 0 */
            if (d == 0) {
                side_effect(o * 100 + i);
            }
            
            /* Also compare original against 1 */
            if (complex_val == 1) {
                side_effect(o * 200 + i);
            }
            
            inner_prev = complex_val % 3;
        }
        
        outer_state = (outer_state + 1) % 2;
    }
}

/* Pattern D: Boolean phi from comparison */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    bool flag = false;
    bool prev_flag = false;
    
    for (int i = 0; i < iterations; i++) {
        /* Create boolean phi: flag = phi(false, !prev_flag) */
        flag = (i == 0) ? false : !prev_flag;
        
        /* Boolean comparison (compiles to == 1 or == 0) */
        if (flag == true) {  /* Will be == 1 */
            side_effect(i + 3000);
        }
        
        /* Inverted comparison */
        if (!flag) {  /* Will be == 0 */
            side_effect(i + 4000);
        }
        
        prev_flag = flag;
    }
}

/* Main hot function that combines all patterns */
__attribute__((noinline, noipa))
void hot_function(int total_iterations) {
    /* Execute each pattern with different weights to create varied profile */
    int part = total_iterations / 4;
    
    /* Pattern A gets 40% of iterations (hot path) */
    pattern_a_loop_phi(part * 2);
    
    /* Pattern B gets 30% */
    pattern_b_merge_phi(part + part/2, 42);
    
    /* Pattern C gets 20% */
    pattern_c_nested_phi(10, part/10);
    
    /* Pattern D gets 10% */
    pattern_d_bool_phi(part/2);
}

/* Checksum function to prevent dead code elimination */
int compute_checksum(void) {
    int sum = global_counter;
    for (int i = 0; i < 1000; i++) {
        sum = (sum * 31 + global_array[i]) & 0xffff;
    }
    return sum;
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
    
    /* Call hot function multiple times to ensure it's annotated as hot */
    for (int run = 0; run < 3; run++) {
        hot_function(iterations);
    }
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum();
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
