/* auto-profile-test.c
 * Program designed to trigger specific uncovered lines in auto-profile.cc
 * lines 1312-1333: GIMPLE_COND with constant RHS, SSA_NAME LHS, phi-node chain
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
    int i;
    
    /* High iteration count to mark as hot loop */
    for (i = 0; i < 100000; ++i) {
        /* Create two SSA variables with trivial operations */
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
        int tmp1 = phi_var;    /* First assignment in chain */
        int tmp2 = tmp1;       /* Second assignment in chain */
        int chain_var = tmp2;  /* Final variable for comparisons */
        
        /* Multiple conditional patterns with constant RHS (0 or 1) */
        
        /* Pattern 1: Equality comparison with 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            /* Trivial work */
            global_counter2 += 2;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* Trivial work */
            global_counter3 += 3;
        }
        
        /* Additional pattern: Not-equal comparison with 1 */
        if (__builtin_expect(chain_var != 1, 1)) {
            /* Mix of operations to prevent simplification */
            global_counter1 ^= 1;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function(int seed) {
    /* Simple function that won't be annotated as hot */
    if (seed > 100) {
        printf("Cold path\n");
    }
}

/* Another hot function with different phi-node structure */
__attribute__((hot))
void second_hot_function(int start, int end) {
    int phi_val;
    int chain1, chain2;
    
    for (int j = start; j < end; ++j) {
        /* Different phi-node creation pattern */
        int x = j * 2;
        int y = j / 2;
        
        /* Phi-node based on bit test */
        if (j & 1) {
            phi_val = x;
        } else {
            phi_val = y;
        }
        
        /* Longer assignment chain */
        int t1 = phi_val;
        int t2 = t1;
        int t3 = t2;
        int t4 = t3;
        chain1 = t4;
        
        /* Comparisons with constants 0 and 1 */
        if (__builtin_expect(chain1 == 0, 0)) {
            global_counter1--;
        }
        
        if (__builtin_expect(chain1 > 1, 1)) {
            global_counter2 <<= 1;
        }
        
        /* Create second phi-node chain */
        int m = j + 5;
        int n = j - 5;
        int phi2 = (j % 3 == 0) ? m : n;
        
        /* Simple assignment chain */
        int s1 = phi2;
        int s2 = s1;
        chain2 = s2;
        
        /* More constant comparisons */
        if (__builtin_expect(chain2 < 1, 0)) {
            global_counter3 |= 0xFF;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call hot function with phi nodes */
    hot_function_with_phi_nodes(base);
    
    /* Call second hot function */
    second_hot_function(0, 50000);
    
    /* Call cold function for contrast */
    cold_function(base);
    
    /* Aggregate results to prevent dead code elimination */
    int result = global_counter1 + global_counter2 + global_counter3;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    return 0;
}
