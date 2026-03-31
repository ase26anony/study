/* Program to trigger auto-profile.cc uncovered lines 1312-1333 */
#include <stdio.h>
#include <stdint.h>

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
        if (i % 10 == 0) {          /* Creates phi at loop header */
            phi_var = a;            /* phi node argument 1 */
        } else {
            phi_var = b;            /* phi node argument 2 */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        int tmp1 = phi_var;         /* First assignment in chain */
        int tmp2 = tmp1;            /* Second assignment in chain */
        int chain_var = tmp2;       /* Final variable for comparisons */
        
        /* Pattern 1: Equality comparison with constant 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
            result += 1;
        }
        
        /* Pattern 2: Greater-than comparison with constant 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            global_counter2++;
            result += 2;
        }
        
        /* Pattern 3: Less-than comparison with constant 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            global_counter3++;
            result += 3;
        }
        
        /* Additional phi-node pattern with different structure */
        int phi_var2;
        if (i % 7 == 0) {
            phi_var2 = a + 0;       /* Add 0 to create trivial assignment */
        } else {
            phi_var2 = b + 0;       /* Add 0 to create trivial assignment */
        }
        
        /* Another assignment chain */
        int chain2_1 = phi_var2;
        int chain2_2 = chain2_1;
        int chain2_var = chain2_2;
        
        /* More comparisons with constants 0 and 1 */
        if (__builtin_expect(chain2_var == 1, 0)) {
            result += 4;
        }
        
        if (__builtin_expect(chain2_var != 0, 1)) {
            result += 5;
        }
    }
    
    return result;
}

/* Cold function to contrast with hot function */
__attribute__((cold))
static void cold_helper_function(void) {
    /* Minimal cold function to establish temperature contrast */
    volatile int cold_var = 0;
    cold_var++;
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;  /* Convert to non-volatile for SSA */
    
    /* Call cold function first */
    cold_helper_function();
    
    /* Call hot function multiple times to ensure profiling */
    int total = 0;
    for (int run = 0; run < 10; ++run) {
        total += hot_function_with_phi_nodes(base + run);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", total);
    printf("Counters: %d, %d, %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return 0;
}
