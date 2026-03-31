/* test_auto_profile.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[1024] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi with comparison against 0 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop in uncovered code */
        int y = x;
        int z = y;
        int w = z;
        
        /* Comparison against 0 - this should trigger the uncovered code */
        if (w == 0) {
            global_counter += 1;
            global_array[i % 1024] = 1;
        } else {
            global_counter -= 1;
            global_array[i % 1024] = 0;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_b(int iterations) {
    /* Pattern B: Merge point phi with comparison against 1 */
    volatile int external = rand() % 100; /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point */
        int val;
        if (external > 50) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Multiple assignments to obscure origin */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against 1 */
        if (tmp2 == 1) {
            global_counter += 2;
            global_array[(i + 512) % 1024] = 2;
        } else {
            global_counter -= 2;
            global_array[(i + 512) % 1024] = 0;
        }
        
        /* Change external to vary branch behavior */
        external = (external * 13 + 7) % 100;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_c(int iterations, bool use_bool) {
    /* Pattern C: Boolean phi with comparison in boolean context */
    bool flag = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi for boolean flag */
        bool new_flag;
        if (i % 3 == 0) {
            new_flag = true;
        } else if (i % 3 == 1) {
            new_flag = false;
        } else {
            new_flag = !flag;
        }
        
        /* Chain of assignments with different types */
        char c = new_flag ? 1 : 0;
        short s = c;
        int tmp = s;
        
        /* Comparison in boolean context (compares against 0/1) */
        if (tmp == 1) {  /* Equivalent to 'if (tmp)' but explicit for coverage */
            global_counter += 3;
            global_array[i % 1024] = 3;
        }
        
        flag = new_flag;
    }
}

__attribute__((noinline, noipa))
void hot_function_nested(int iterations) {
    /* More complex pattern with nested loops and multiple phis */
    int outer_acc = 0;
    
    for (int outer = 0; outer < 10; ++outer) {
        int inner_acc = outer;
        
        for (int inner = 0; inner < iterations / 10; ++inner) {
            /* Phi from loop header and previous iteration */
            int phi_val = (inner == 0) ? outer : inner_acc;
            
            /* Multiple chained assignments */
            int a = phi_val;
            int b = a;
            int c = b;
            
            /* Multiple comparisons against 0 and 1 */
            if (c == 0) {
                global_counter += outer;
                global_array[inner % 1024] = outer;
            }
            
            if (c == 1) {
                global_counter -= outer;
                global_array[(inner + 256) % 1024] = -outer;
            }
            
            inner_acc = phi_val + 1;
        }
        
        outer_acc += inner_acc;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default large number for hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Seed random for pattern B */
    srand(42);
    
    /* Execute all patterns to generate diverse profile data */
    hot_function_pattern_a(iterations, 0);  /* Start with 0 to hit the == 0 branch */
    hot_function_pattern_b(iterations);
    hot_function_pattern_c(iterations / 2, true);
    hot_function_nested(iterations);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
