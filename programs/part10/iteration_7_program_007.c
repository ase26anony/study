/* auto-profile-test.c
 * Program designed to trigger uncovered lines 1312-1333 in auto-profile.cc
 * Compile with: gcc -O2 -fauto-profile -fprofile-arcs -fdump-ipa-afdo auto-profile-test.c -o auto-profile-test
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
    
    /* Main hot loop with high iteration count */
    for (i = 0; i < 100000; ++i) {
        /* Create two SSA variables from base and loop counter */
        int a = base_value + i;      /* SSA variable a */
        int b = base_value - i;      /* SSA variable b */
        
        /* Create phi-node candidate: conditional assignment creates phi in GIMPLE */
        int phi_var;
        if (i % 10 == 0) {
            phi_var = a;            /* phi-node edge 1 */
        } else {
            phi_var = b;            /* phi-node edge 2 */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        int tmp1 = phi_var;         /* First assignment in chain */
        int tmp2 = tmp1;            /* Second assignment in chain */
        int chain_var = tmp2;       /* Final variable for comparisons */
        
        /* Pattern 1: Equality comparison with constant 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with constant 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            /* Trivial work to keep code live */
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with constant 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* Trivial work to keep code live */
            global_counter3++;
        }
        
        /* Additional pattern: Not-equal comparison with constant 1 */
        if (__builtin_expect(chain_var != 1, 1)) {
            /* More trivial work */
            global_counter1 += 0;  /* Add 0 to avoid being optimized out */
        }
    }
}

/* Cold function attribute for contrast */
__attribute__((cold))
void cold_function() {
    /* This function is marked cold to help auto-profile distinguish hot/cold regions */
    printf("Cold function (should not affect hot loop profiling)\n");
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call cold function first */
    cold_function();
    
    /* Call hot function multiple times to ensure it's profiled as hot */
    hot_function_with_phi_nodes(base);
    hot_function_with_phi_nodes(base + 1);
    hot_function_with_phi_nodes(base - 1);
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total counters: %d\n", total);
    printf("Counter1: %d, Counter2: %d, Counter3: %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return 0;
}
