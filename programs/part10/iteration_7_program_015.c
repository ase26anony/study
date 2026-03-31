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
    int i;
    
    /* Main loop with high iteration count */
    for (i = 0; i < 100000; ++i) {
        /* Create two SSA variables from base and loop counter */
        int a = base_value + i;      /* SSA variable a */
        int b = base_value - i;      /* SSA variable b */
        
        /* Create phi-node candidate: conditional assignment */
        int phi_var;
        if (i % 10 == 0) {
            phi_var = a;            /* Creates phi-node in GIMPLE */
        } else {
            phi_var = b;            /* Other phi-node argument */
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
            /* Another trivial operation */
            global_counter2 += 2;
        }
        
        /* Pattern 3: Less-than comparison with constant 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* More trivial work */
            global_counter3 += 3;
        }
        
        /* Additional pattern: Not-equal comparison with constant 1 */
        if (__builtin_expect(chain_var != 1, 1)) {
            global_counter1 += chain_var & 1;
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
    /* Use volatile seed to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call cold function once */
    cold_function();
    
    /* Call hot function multiple times to establish profile */
    hot_function_with_phi_nodes(base);
    hot_function_with_phi_nodes(base + 1);
    hot_function_with_phi_nodes(base - 1);
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total counters: %d\n", total);
    
    return 0;
}
