/* Program to trigger auto-profile.cc uncovered lines 1312-1333
 * Creates GIMPLE_COND with constant RHS (0/1), SSA_NAME LHS,
 * phi-node dependency chains, and annotated basic blocks
 */

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
        /* Create two SSA variables with trivial arithmetic */
        a = base_value + i;      /* SSA_NAME */
        b = base_value - i;      /* SSA_NAME */
        
        /* Create phi-node: conditionally assign a or b */
        /* This generates a phi statement in GIMPLE */
        if (i % 10 == 0) {
            phi_var = a;         /* Creates phi-node at loop header */
        } else {
            phi_var = b;         /* Alternative phi-node argument */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        /* tmp1 = phi_var (single assignment, RHS is SSA_NAME) */
        tmp1 = phi_var;
        
        /* tmp2 = tmp1 (another single assignment) */
        tmp2 = tmp1;
        
        /* tmp3 = tmp2 (another single assignment) */
        tmp3 = tmp2;
        
        /* Final chain_var = tmp3 */
        chain_var = tmp3;
        
        /* Pattern 1: Equality comparison with 0 */
        /* GIMPLE_COND with constant RHS = 0, LHS = SSA_NAME (chain_var) */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        /* GIMPLE_COND with constant RHS = 1, LHS = SSA_NAME (chain_var) */
        if (__builtin_expect(chain_var > 1, 1)) {
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        /* GIMPLE_COND with constant RHS = 1, LHS = SSA_NAME (chain_var) */
        if (__builtin_expect(chain_var < 1, 0)) {
            global_counter3++;
        }
        
        /* Additional pattern: Not-equal comparison with 0 */
        if (__builtin_expect(chain_var != 0, 1)) {
            global_counter1 += 0; /* Minimal work */
        }
        
        /* Pattern with RHS = 1 using equality */
        if (__builtin_expect(chain_var == 1, 0)) {
            global_counter2 += 0;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function() {
    /* Minimal cold function for annotation contrast */
    printf("Cold function executed\n");
}

/* Another hot function with different phi-node structure */
__attribute__((hot))
void second_hot_function(int start, int end) {
    int x, y, z, result;
    
    for (int i = start; i < end; ++i) {
        /* Different phi-node creation pattern */
        x = i * 2;
        y = i / 2;
        
        /* Phi-node based on bit test */
        if (i & 1) {
            z = x;
        } else {
            z = y;
        }
        
        /* Assignment chain */
        int chain1 = z;
        int chain2 = chain1;
        int final_var = chain2;
        
        /* Multiple constant comparisons */
        if (__builtin_expect(final_var == 0, 0)) {
            global_counter1++;
        }
        
        if (__builtin_expect(final_var > 1, 1)) {
            global_counter2++;
        }
        
        if (__builtin_expect(final_var < 1, 0)) {
            global_counter3++;
        }
    }
}

int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;  /* Convert to non-volatile for use */
    
    /* Call hot function with phi nodes */
    hot_function_with_phi_nodes(base);
    
    /* Call second hot function */
    second_hot_function(0, 50000);
    
    /* Call cold function for contrast */
    cold_function();
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total operations: %d\n", total);
    printf("Counters: %d, %d, %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return total > 0 ? 0 : 1;
}
