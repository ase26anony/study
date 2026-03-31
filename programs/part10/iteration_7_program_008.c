/* Program to trigger auto-profile.cc uncovered lines 1312-1333 */
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
        /* Create two SSA variables from base and loop counter */
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
        /* This creates the GIMPLE_ASSIGN chain the code traces through */
        tmp1 = phi_var;         /* First assignment: tmp1 = phi_var (SSA_NAME) */
        tmp2 = tmp1;            /* Second assignment: tmp2 = tmp1 (SSA_NAME) */
        tmp3 = tmp2;            /* Third assignment: tmp3 = tmp2 (SSA_NAME) */
        chain_var = tmp3;       /* Final chain variable */
        
        /* Pattern 1: Equality comparison with constant 0 */
        /* cmp_rhs = 0 (TREE_CONSTANT, integer_zerop) */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with constant 1 */
        /* cmp_rhs = 1 (TREE_CONSTANT, integer_onep) */
        if (__builtin_expect(chain_var > 1, 1)) {
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with constant 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            global_counter3++;
        }
        
        /* Additional pattern: Not-equal comparison with 0 */
        if (__builtin_expect(chain_var != 0, 1)) {
            global_counter1 += 0;  /* Minimal work */
        }
        
        /* Pattern with different constant 1 comparison */
        if (__builtin_expect(chain_var == 1, 0)) {
            global_counter2 += 0;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function(int x) {
    /* Simple cold function - not expected to be annotated */
    if (x == 0) {
        printf("Cold\n");
    }
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call hot function multiple times to ensure profiling */
    for (int run = 0; run < 10; ++run) {
        hot_function_with_phi_nodes(base + run);
    }
    
    /* Call cold function for contrast */
    cold_function(0);
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total operations: %d\n", total);
    printf("Counters: %d, %d, %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return total > 0 ? 0 : 1;
}

/* Additional function with different phi-node pattern */
__attribute__((hot))
void another_hot_function(int start) {
    int x, y, z;
    
    for (int i = 0; i < 50000; ++i) {
        /* Alternative phi-node creation */
        x = start + i * 2;
        y = start - i * 2;
        
        /* Phi via ternary operator (should also create phi-node) */
        z = (i & 1) ? x : y;
        
        /* Chain of assignments */
        int t1 = z;
        int t2 = t1;
        int final_var = t2;
        
        /* Multiple constant comparisons */
        if (__builtin_expect(final_var == 0, 0)) {
            global_counter1++;
        }
        if (__builtin_expect(final_var > 1, 1)) {
            global_counter2++;
        }
    }
}
