/* Program to trigger auto-profile.cc lines 1312-1333 */
#include <stdio.h>
#include <stdlib.h>

/* Global counters to prevent optimization */
volatile int global_counter1 = 0;
volatile int global_counter2 = 0;
volatile int global_counter3 = 0;

/* Hot function attribute to encourage auto-profile annotation */
__attribute__((hot)) 
void hot_function_with_phi_nodes(int base_value) {
    int a, b, phi_var, chain_var;
    int tmp1, tmp2, tmp3;
    
    /* High iteration count to mark as hot loop */
    for (int i = 0; i < 100000; ++i) {
        /* Create two SSA variables with trivial operations */
        a = base_value + i;      /* SSA variable a */
        b = base_value - i;      /* SSA variable b */
        
        /* Create phi-node: conditionally assign a or b */
        /* This generates a phi statement in GIMPLE */
        if (i % 10 == 0) {
            phi_var = a;        /* One incoming edge to phi */
        } else {
            phi_var = b;        /* Other incoming edge to phi */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        /* This creates the GIMPLE_ASSIGN chain the code traces */
        tmp1 = phi_var;         /* First assignment: SSA_NAME <- SSA_NAME */
        tmp2 = tmp1;            /* Second assignment: SSA_NAME <- SSA_NAME */
        tmp3 = tmp2;            /* Third assignment: SSA_NAME <- SSA_NAME */
        chain_var = tmp3;       /* Final variable for comparisons */
        
        /* Multiple conditional patterns with constant RHS (0 or 1) */
        /* Pattern 1: Equality comparison with 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            /* Trivial work to keep code live */
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* Trivial work to keep code live */
            global_counter3++;
        }
        
        /* Pattern 4: Equality comparison with 1 */
        if (__builtin_expect(chain_var == 1, 0)) {
            /* Additional pattern for coverage */
            global_counter1 += 2;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function() {
    /* Minimal cold function for profile contrast */
    printf("Cold function executed\n");
}

int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call hot function multiple times to ensure profiling */
    for (int run = 0; run < 10; ++run) {
        hot_function_with_phi_nodes(base + run);
    }
    
    /* Call cold function once for contrast */
    cold_function();
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total counters: %d\n", total);
    printf("Counter1: %d, Counter2: %d, Counter3: %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return 0;
}
