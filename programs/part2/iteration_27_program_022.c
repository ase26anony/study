/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi node analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Function to create phi nodes from loop conditions - Pattern A */
__attribute__((noinline, noipa))
void pattern_a_loop_phi(int iterations) {
    int prev_x = 0;
    int phi_val;
    
    for (int i = 0; i < iterations; i++) {
        /* Create a phi node: phi_val = (i == 0) ? 1 : prev_x */
        if (i == 0) {
            phi_val = 1;
        } else {
            phi_val = prev_x;
        }
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int temp1 = phi_val;      /* GIMPLE_ASSIGN 1 */
        int temp2 = temp1;        /* GIMPLE_ASSIGN 2 */
        int temp3 = temp2;        /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived value compared to 0 */
        if (temp3 == 0) {         /* Should trigger uncovered lines */
            global_counter += 1;
            global_array[i % 1024] = i;
        }
        
        /* Another comparison with 1 */
        int temp4 = phi_val;
        if (temp4 == 1) {         /* Another trigger for uncovered lines */
            global_counter -= 1;
        }
        
        /* Update for next iteration to create loop-carried dependency */
        prev_x = (phi_val + i) % 3;
    }
}

/* Function with merge point phi - Pattern B */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = iterations; /* Prevent constant folding */
    
    for (int i = 0; i < iterations; i++) {
        /* Create a phi at merge point */
        int merge_val;
        if ((seed + i) % 2 == 0) {
            merge_val = 1;
        } else {
            merge_val = 0;
        }
        
        /* Multiple assignments to obscure origin */
        int chain1 = merge_val;
        int chain2 = chain1;
        
        /* Comparison against 1 */
        if (chain2 == 1) {        /* Should trigger uncovered lines */
            global_array[(i + 1) % 1024] += 1;
        }
        
        /* Another comparison against 0 */
        if (merge_val == 0) {     /* Direct use of phi */
            global_counter *= 2;
            if (global_counter > 1000000) global_counter = 1;
        }
    }
}

/* Complex pattern with nested loops and multiple phi nodes */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int outer_iter, int inner_iter) {
    int outer_phi = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        /* Phi from loop header */
        int loop_phi = (o == 0) ? 1 : outer_phi;
        
        for (int i = 0; i < inner_iter; i++) {
            /* Nested phi with dependency on both loops */
            int nested_phi;
            if (i == 0) {
                nested_phi = loop_phi;
            } else {
                nested_phi = (nested_phi + 1) % 2;
            }
            
            /* Chain of assignments */
            int a = nested_phi;
            int b = a;
            int c = b;
            
            /* Multiple comparisons to increase coverage */
            if (c == 0) {
                global_counter += o;
            }
            
            if (a == 1) {
                global_counter -= i;
            }
            
            /* Use volatile to prevent optimization */
            volatile int guard = c;
            if (guard == 0) {
                global_array[(o * inner_iter + i) % 1024] = guard;
            }
        }
        
        outer_phi = (loop_phi + 1) % 2;
    }
}

/* Function using boolean phi nodes */
__attribute__((noinline, noipa))
void pattern_d_bool_phi(int iterations) {
    bool flag = true;
    
    for (int i = 0; i < iterations; i++) {
        /* Boolean phi node */
        bool bool_phi;
        if (i % 3 == 0) {
            bool_phi = true;    /* true becomes 1 */
        } else {
            bool_phi = flag;
        }
        
        /* Comparison with boolean constant (compiles to == 1) */
        if (bool_phi == true) {  /* Should become comparison with 1 */
            global_counter += 2;
        }
        
        /* Inverted comparison */
        if (!bool_phi) {         /* Should become comparison with 0 */
            global_array[i % 1024] = i;
        }
        
        flag = !flag;
    }
}

/* Main hot function that calls all patterns */
__attribute__((noinline, noipa))
void hot_function(int total_iterations) {
    /* Mix different patterns to create various phi scenarios */
    int part = total_iterations / 4;
    
    pattern_a_loop_phi(part);          /* Pattern A */
    pattern_b_merge_phi(part);         /* Pattern B */
    pattern_c_complex_phi(100, part/100); /* Pattern C */
    pattern_d_bool_phi(part);          /* Pattern D */
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Starting AutoFDO test with %d iterations\n", iterations);
    
    /* Execute hot function multiple times to ensure profiling */
    for (int run = 0; run < 10; run++) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}
