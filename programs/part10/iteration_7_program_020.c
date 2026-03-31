/* Program to trigger auto-profile.cc lines 1312-1333 */
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
        int a = base_value + i;
        int b = base_value - i;
        
        /* Create phi-node: conditionally assign a or b */
        int phi_var;
        if (i % 10 == 0) {
            phi_var = a;  /* This creates a phi node in GIMPLE */
        } else {
            phi_var = b;  /* This creates a phi node in GIMPLE */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        int tmp1 = phi_var;
        int tmp2 = tmp1;
        int chain_var = tmp2;  /* Final SSA_NAME for comparisons */
        
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
            result += chain_var;
        }
    }
    
    return result;
}

/* Cold function to contrast with hot function */
__attribute__((cold))
static void cold_function() {
    /* Minimal cold code */
    printf("Cold function executed\n");
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Add some trivial arithmetic to create SSA complexity */
    base = base + 0;
    base = base - 0;
    base = base * 1;
    
    /* Call hot function multiple times */
    int total = 0;
    for (int j = 0; j < 10; ++j) {
        total += hot_function_with_phi_nodes(base + j);
    }
    
    /* Call cold function occasionally */
    if (total % 1000 == 0) {
        cold_function();
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", total);
    printf("Counters: %d, %d, %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return 0;
}
