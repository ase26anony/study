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
            phi_var = a;            /* Forms phi-node in GIMPLE */
        } else {
            phi_var = b;            /* Alternative phi-node input */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        int tmp1 = phi_var;         /* First assignment in chain */
        int tmp2 = tmp1;            /* Second assignment in chain */
        int chain_var = tmp2;       /* Final variable for comparisons */
        
        /* Pattern 1: Equality comparison with 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1 += 1;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            global_counter2 += chain_var;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            global_counter3 -= chain_var;
        }
        
        /* Additional phi-node pattern with different structure */
        int phi_var2;
        int c = base_value * 2 + i;
        int d = base_value / 2 - i;
        
        /* Another phi-node via ternary operator */
        phi_var2 = (i % 5 == 0) ? c : d;
        
        /* Chain for second phi variable */
        int tmp3 = phi_var2;
        int tmp4 = tmp3;
        int chain_var2 = tmp4;
        
        /* More comparisons with constants 0 and 1 */
        if (__builtin_expect(chain_var2 == 1, 0)) {
            global_counter1 += 2;
        }
        
        if (__builtin_expect(chain_var2 > 0, 1)) {
            global_counter2 += 3;
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_function() {
    /* Minimal cold code */
    printf("Cold function executed\n");
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Call hot function multiple times */
    hot_function_with_phi_nodes(base);
    
    /* Add some variation */
    if (argc > 1) {
        base = atoi(argv[1]);
    }
    
    hot_function_with_phi_nodes(base + 1);
    
    /* Call cold function */
    cold_function();
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total: %d (Counters: %d, %d, %d)\n", 
           total, global_counter1, global_counter2, global_counter3);
    
    return total != 0 ? 0 : 1;
}
