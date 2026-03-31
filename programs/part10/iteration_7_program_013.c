/* Program to trigger auto-profile.cc uncovered lines 1312-1333 */
#include <stdio.h>
#include <stdlib.h>

/* Global counters to prevent optimization */
volatile int global_counter1 = 0;
volatile int global_counter2 = 0;
volatile int global_counter3 = 0;

/* Hot function attribute to encourage auto-profile annotation */
__attribute__((hot)) 
static int hot_function_with_phi_nodes(int base_value) {
    int result = 0;
    
    /* High iteration count to mark as hot loop */
    for (int i = 0; i < 100000; ++i) {
        /* Create two SSA variables from base and loop counter */
        int a = base_value + i;      /* SSA variable a */
        int b = base_value - i;      /* SSA variable b */
        
        /* Create phi-node: conditionally assign a or b */
        int phi_var;
        if (i % 10 == 0) {
            phi_var = a;  /* One incoming edge to phi */
        } else {
            phi_var = b;  /* Other incoming edge to phi */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        /* This creates the GIMPLE_ASSIGN chain the code traces back through */
        int tmp1 = phi_var;      /* First assignment: tmp1 = phi_var */
        int tmp2 = tmp1;         /* Second: tmp2 = tmp1 */
        int chain_var = tmp2;    /* Third: chain_var = tmp2 */
        
        /* Multiple conditional patterns with constant RHS (0 or 1) */
        /* Pattern 1: Equality comparison with 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
            result += 1;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            global_counter2++;
            result += 2;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            global_counter3++;
            result += 3;
        }
        
        /* Additional pattern: Not-equal comparison with 1 */
        if (__builtin_expect(chain_var != 1, 1)) {
            result += 4;
        }
    }
    
    return result;
}

/* Cold function attribute for contrast */
__attribute__((cold))
static void cold_helper_function(void) {
    /* This function is marked cold to help auto-profile distinguish hot/cold regions */
    volatile int x = 0;
    for (int i = 0; i < 10; ++i) {
        x += i;
    }
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call cold function first */
    cold_helper_function();
    
    /* Call hot function multiple times to ensure profiling */
    int total = 0;
    for (int j = 0; j < 10; ++j) {
        total += hot_function_with_phi_nodes(base + j);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result: %d\n", total);
    printf("Counters: %d, %d, %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return 0;
}
