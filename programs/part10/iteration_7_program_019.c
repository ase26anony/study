/* Program to trigger auto-profile.cc lines 1312-1333 */
#include <stdio.h>
#include <stdint.h>

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
        
        /* Create phi-node: phi_var gets either a or b based on condition */
        /* This generates a phi statement in GIMPLE */
        if (i % 10 == 0) {
            phi_var = a;        /* phi-node edge 1 */
        } else {
            phi_var = b;        /* phi-node edge 2 */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        /* This creates the GIMPLE_ASSIGN chain the uncovered code traces */
        tmp1 = phi_var;         /* First assignment in chain */
        tmp2 = tmp1;            /* Second assignment in chain */
        tmp3 = tmp2;            /* Third assignment in chain */
        chain_var = tmp3;       /* Final variable for comparisons */
        
        /* Pattern 1: Equality comparison with constant 0 */
        /* Generates GIMPLE_COND with RHS = 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with constant 1 */
        /* Generates GIMPLE_COND with RHS = 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            /* Trivial work to keep code live */
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with constant 1 */
        /* Generates GIMPLE_COND with RHS = 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* Trivial work to keep code live */
            global_counter3++;
        }
        
        /* Pattern 4: Not-equal comparison with constant 0 */
        /* Another GIMPLE_COND with RHS = 0 */
        if (__builtin_expect(chain_var != 0, 1)) {
            global_counter1 += 2;
        }
        
        /* Pattern 5: Equality comparison with constant 1 */
        if (__builtin_expect(chain_var == 1, 0)) {
            global_counter2 += 2;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function() {
    /* Minimal cold function to establish profile contrast */
    printf("Cold function executed\n");
}

int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call cold function once */
    cold_function();
    
    /* Call hot function multiple times to establish hot profile */
    for (int run = 0; run < 10; ++run) {
        hot_function_with_phi_nodes(base + run);
    }
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total counters: %d\n", total);
    printf("Counter1: %d, Counter2: %d, Counter3: %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return total > 0 ? 0 : 1;
}
