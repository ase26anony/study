/* test_auto_profile.c - Test program for GCC AutoFDO phi-node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Function to create side effects - prevents dead code elimination */
__attribute__((noinline)) void side_effect(int idx, int val) {
    global_array[idx & 1023] ^= val;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
__attribute__((noinline, noipa)) 
void pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int temp1, temp2, temp3;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi node: x is phi(0, x_prev + 1) */
        int x = (i == 0) ? 0 : prev + 1;
        
        /* Chain of assignments to obscure the phi origin */
        temp1 = x;      /* GIMPLE_ASSIGN 1 */
        temp2 = temp1;  /* GIMPLE_ASSIGN 2 */
        temp3 = temp2;  /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 (RHS is 0) */
        if (temp3 == 0) {
            side_effect(i, 1);
        }
        
        /* Another comparison against constant 1 */
        if (x == 1) {
            side_effect(i, 2);
        }
        
        prev = x;
    }
}

/* Pattern B: Merge point phi from conditional assignment */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = iterations; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi at merge point of two paths */
        int cond_val;
        if (seed & 1) {
            cond_val = 1;
        } else {
            cond_val = 0;
        }
        
        /* Use the phi result in comparison against 1 */
        if (cond_val == 1) {
            side_effect(i, 3);
        }
        
        /* Chain assignments to test the while loop walking back */
        int chain1 = cond_val;
        int chain2 = chain1;
        if (chain2 == 0) {
            side_effect(i, 4);
        }
        
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
    }
}

/* Pattern C: Nested loop with complex phi structure */
__attribute__((noinline, noipa))
void pattern_c_nested_phi(int outer_iter, int inner_iter) {
    int state = 0;
    
    for (int o = 0; o < outer_iter; ++o) {
        int inner_state = (o == 0) ? 0 : state;
        
        for (int i = 0; i < inner_iter; ++i) {
            /* phi: inner_state merges previous iteration's value */
            int current = (i == 0) ? inner_state : (inner_state + i) % 3;
            
            /* Multiple chained assignments */
            int a = current;
            int b = a;
            int c = b;
            
            /* Comparisons against both 0 and 1 */
            if (c == 0) {
                side_effect(o * inner_iter + i, 5);
            }
            
            if (current == 1) {
                side_effect(o * inner_iter + i, 6);
            }
            
            inner_state = current;
        }
        
        state = inner_state;
    }
}

/* Pattern D: Boolean phi from comparison */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    volatile int threshold = 1000;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create boolean phi */
        bool flag;
        if (i < threshold) {
            flag = true;  /* becomes 1 */
        } else {
            flag = false; /* becomes 0 */
        }
        
        /* Boolean comparison - should become == 1 */
        if (flag == true) {
            side_effect(i, 7);
        }
        
        /* Another comparison against 0 */
        bool flag2 = !flag;
        if (flag2 == false) {  /* becomes == 0 */
            side_effect(i, 8);
        }
    }
}

/* Main hot function that combines all patterns */
__attribute__((noinline, noipa))
void hot_function(int total_iterations) {
    /* Mix different patterns to create various phi structures */
    int quarter = total_iterations / 4;
    
    pattern_a_loop_phi(quarter);
    pattern_b_merge_phi(quarter);
    pattern_c_nested_phi(10, quarter / 10);
    pattern_d_bool_phi(quarter);
    
    /* Additional mixed pattern to ensure coverage */
    volatile int mix_seed = 42;
    for (int i = 0; i < quarter; ++i) {
        /* Complex phi with multiple predecessors */
        int val;
        switch (mix_seed % 4) {
            case 0: val = 0; break;
            case 1: val = 1; break;
            case 2: val = 2; break;
            case 3: val = 3; break;
        }
        
        /* Chain of 4 assignments */
        int v1 = val;
        int v2 = v1;
        int v3 = v2;
        int v4 = v3;
        
        /* Target comparisons */
        if (v4 == 0) {
            side_effect(i, 9);
        }
        if (v4 == 1) {
            side_effect(i, 10);
        }
        
        mix_seed = (mix_seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default - should be hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function multiple times to ensure profiling */
    for (int run = 0; run < 3; ++run) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum ^= global_array[i];
    }
    
    printf("Checksum: %d, Side effects: %d\n", checksum, global_counter);
    
    return checksum != 0 ? 0 : 1;
}
