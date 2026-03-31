/* test_auto_profile.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int results[1000] = {0};

/* Function to create phi nodes from loop conditions */
__attribute__((noinline, noipa))
void hot_function_loop_phi(int iterations, int start_val) {
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern A: Loop-dependent phi node */
        /* 'x' becomes a phi node merging values from:
           - Initial value (when i == 0)
           - Previous iteration value + 1 (when i > 0) */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Insert chained assignments to test the while loop in auto-profile.cc */
        /* Pattern C: Chained copies */
        int a = x;      /* First copy */
        int b = a;      /* Second copy */
        int c = b;      /* Third copy */
        
        /* Comparison against constant 0 */
        if (c == 0) {   /* This should trigger the uncovered code */
            results[global_counter % 1000] += 1;
            global_counter++;
        }
        
        /* Another comparison against constant 1 */
        int d = (x % 2 == 0) ? 1 : 0;  /* Pattern B: Merge point phi */
        int e = d;                     /* Another copy */
        if (e == 1) {                  /* Comparison against 1 */
            results[global_counter % 1000] += 2;
            global_counter++;
        }
        
        x_prev = x;
    }
}

/* Another function with different phi patterns */
__attribute__((noinline, noipa))
void hot_function_conditional_phi(int iterations) {
    volatile int external = rand() % 100;  /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern B: Conditional merge phi */
        int cond_val = (external > 50) ? 1 : 0;
        
        /* Multiple chained assignments */
        int tmp1 = cond_val;
        int tmp2 = tmp1;
        int tmp3 = tmp2;
        
        /* Comparison that should be annotated */
        if (tmp3 == 1) {
            results[global_counter % 1000] += 3;
            global_counter += 2;
        }
        
        /* Nested condition creating more phi nodes */
        int nested_val;
        if (i % 3 == 0) {
            nested_val = 0;
        } else if (i % 3 == 1) {
            nested_val = 1;
        } else {
            nested_val = (external % 2);
        }
        
        /* More chained copies */
        int chain1 = nested_val;
        int chain2 = chain1;
        if (chain2 == 0) {
            results[global_counter % 1000] += 5;
            global_counter += 3;
        }
        
        /* Update external to create varying behavior */
        external = (external * 13 + 17) % 100;
    }
}

/* Function with boolean phi nodes */
__attribute__((noinline, noipa))
void hot_function_bool_phi(int iterations) {
    _Bool flag1 = 0;
    _Bool flag2 = 1;
    
    for (int i = 0; i < iterations; ++i) {
        /* Boolean phi node from loop */
        _Bool current_flag = (i % 10 == 0) ? flag2 : flag1;
        
        /* Boolean comparison (compiles to == 1 or == 0) */
        if (current_flag == 1) {  /* Should create comparison with constant 1 */
            results[global_counter % 1000] += 7;
            global_counter += 5;
        }
        
        /* Toggle flags to create phi merges */
        if (i % 7 == 0) {
            flag1 = !flag1;
            flag2 = !flag2;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    /* Seed random for varying behavior */
    srand(42);
    
    /* Reset global counter */
    global_counter = 0;
    memset(results, 0, sizeof(results));
    
    /* Execute hot functions to create profile data */
    hot_function_loop_phi(iterations / 3, 0);
    hot_function_conditional_phi(iterations / 3);
    hot_function_bool_phi(iterations / 3);
    
    /* Additional mixed execution to ensure all paths are taken */
    for (int i = 0; i < iterations / 10; ++i) {
        hot_function_loop_phi(10, i % 5);
        hot_function_conditional_phi(5);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1000; ++i) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
