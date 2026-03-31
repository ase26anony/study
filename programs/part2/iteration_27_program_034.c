/* test_auto_profile.c - Test program for GCC AutoFDO coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Function to create phi node from loop with chained assignments */
__attribute__((noinline, noipa))
void hot_loop_phi(int iterations, int start_val) {
    int prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern A: Loop-dependent phi node */
        /* 'x' becomes a phi node merging values from:
           - Initial value (when i == 0)
           - Previous iteration value + 1 (when i > 0) */
        int x;
        if (i == 0) {
            x = start_val;
        } else {
            x = prev + 1;
        }
        
        /* Pattern C: Chained assignments to obscure origin */
        int a = x;      /* First copy */
        int b = a;      /* Second copy */
        int c = b;      /* Third copy - this is what we compare */
        
        /* Comparison against constant 0 */
        if (c == 0) {
            /* Hot path - executed many times */
            global_array[i % 1024] += 1;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            global_array[i % 1024] -= 1;
        }
        
        /* Comparison against constant 1 */
        if (c == 1) {
            /* Another hot path */
            global_array[(i + 1) % 1024] *= 2;
        }
        
        prev = x;
    }
}

/* Function to create phi node at conditional merge point */
__attribute__((noinline, noipa))
void conditional_merge_phi(int iterations) {
    volatile int seed = 42; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern B: Merge point phi */
        /* 'val' becomes a phi node merging 1 and 0 from two branches */
        int val;
        if ((seed + i) % 100 < 90) { /* 90% probability for hot path */
            val = 1;  /* Hot branch */
        } else {
            val = 0;  /* Cold branch */
        }
        
        /* Direct comparison of phi result against 1 */
        if (val == 1) {
            /* Hot path */
            global_counter += 2;
            global_array[i % 1024] = i;
        }
        
        /* Add some chained copies */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against 0 through chained copies */
        if (tmp2 == 0) {
            /* Cold path */
            global_counter -= 1;
        }
    }
}

/* Complex pattern with nested loops and multiple phi nodes */
__attribute__((noinline, noipa))
void complex_nested_phi(int outer_iter, int inner_iter) {
    int outer_acc = 0;
    
    for (int o = 0; o < outer_iter; ++o) {
        int inner_state = o % 3;
        
        for (int i = 0; i < inner_iter; ++i) {
            /* Phi node from loop header */
            int loop_phi;
            if (i == 0) {
                loop_phi = inner_state;
            } else {
                loop_phi = (inner_state + i) % 2;
            }
            
            /* Multiple chained assignments */
            int chain1 = loop_phi;
            int chain2 = chain1;
            int chain3 = chain2;
            
            /* Multiple comparisons against 0 and 1 */
            if (chain3 == 0) {
                global_array[(o * inner_iter + i) % 1024] += o;
            }
            
            if (chain2 == 1) {
                global_counter += i;
            }
            
            /* Boolean phi from conditional */
            _Bool flag;
            if (loop_phi > 0) {
                flag = 1;  /* true */
            } else {
                flag = 0;  /* false */
            }
            
            /* Boolean comparison (compiles to == 1) */
            if (flag == 1) {
                outer_acc++;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    global_array[0] += outer_acc;
}

/* Main function with configurable iterations */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - large enough to be hot */
    
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
    
    /* Execute hot functions to generate profile data */
    hot_loop_phi(iterations, 0);          /* Start with 0 to hit == 0 path */
    conditional_merge_phi(iterations / 2);
    complex_nested_phi(100, iterations / 100);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
