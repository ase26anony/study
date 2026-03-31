/* Program to trigger auto-profile.cc uncovered lines 1312-1333 */
#include <stdio.h>
#include <stdlib.h>

/* Global counters to prevent optimization */
volatile int global_counter1 = 0;
volatile int global_counter2 = 0;
volatile int global_counter3 = 0;

/* Hot function attribute to encourage auto-profile annotation */
__attribute__((hot))
void hot_loop_function(int base_value) {
    int a, b, phi_var, tmp1, tmp2, chain_var;
    
    /* High iteration count to mark as hot loop */
    for (int i = 0; i < 100000; ++i) {
        /* Create two SSA variables with trivial operations */
        a = base_value + i;      /* SSA_NAME: a */
        b = base_value - i;      /* SSA_NAME: b */
        
        /* Create phi-node: conditionally assign a or b */
        /* This generates a gphi node in GIMPLE */
        if (i % 10 == 0) {
            phi_var = a;         /* phi-node candidate */
        } else {
            phi_var = b;         /* phi-node candidate */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        /* This creates the GIMPLE_ASSIGN chain the code traces through */
        tmp1 = phi_var;          /* First assignment in chain */
        tmp2 = tmp1;             /* Second assignment in chain */
        chain_var = tmp2;        /* Final variable for comparisons */
        
        /* Pattern 1: Equality comparison with 0 */
        /* Generates GIMPLE_COND with constant RHS 0 */
        if (__builtin_expect(chain_var == 0, 0)) {
            /* Trivial work to keep code live */
            global_counter1++;
        }
        
        /* Pattern 2: Greater-than comparison with 1 */
        /* Generates GIMPLE_COND with constant RHS 1 */
        if (__builtin_expect(chain_var > 1, 1)) {
            /* Trivial work to keep code live */
            global_counter2++;
        }
        
        /* Pattern 3: Less-than comparison with 1 */
        /* Generates GIMPLE_COND with constant RHS 1 */
        if (__builtin_expect(chain_var < 1, 0)) {
            /* Trivial work to keep code live */
            global_counter3++;
        }
        
        /* Additional pattern: Not-equal comparison with 0 */
        /* Another GIMPLE_COND with constant RHS 0 */
        if (__builtin_expect(chain_var != 0, 1)) {
            global_counter1 += 0; /* Minimal work */
        }
    }
}

/* Cold function to contrast with hot function */
__attribute__((cold))
void cold_helper(int x) {
    /* Simple cold function to provide contrast */
    if (x > 100) {
        printf("Cold path taken\n");
    }
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int seed = 42;
    int base = seed;
    
    /* Add some input dependence */
    if (argc > 1) {
        base += atoi(argv[1]);
    }
    
    /* Call hot function multiple times to ensure profiling */
    for (int run = 0; run < 10; ++run) {
        hot_loop_function(base + run);
    }
    
    /* Call cold function occasionally */
    if (base % 100 == 0) {
        cold_helper(base);
    }
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = global_counter1 + global_counter2 + global_counter3;
    printf("Total counters: %d\n", total);
    printf("Counter1: %d, Counter2: %d, Counter3: %d\n", 
           global_counter1, global_counter2, global_counter3);
    
    return total > 0 ? 0 : 1;
}

/* Additional function to create more control flow complexity */
__attribute__((hot))
void another_hot_path(int iterations) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi-node with different structure */
        int source1 = i * 2;
        int source2 = i / 2;
        int phi_val = (i % 3 == 0) ? source1 : source2;
        
        /* Chain assignments */
        int intermediate = phi_val;
        int final_var = intermediate;
        
        /* Multiple constant comparisons */
        if (__builtin_expect(final_var == 1, 0)) {
            x++;
        }
        if (__builtin_expect(final_var > 0, 1)) {
            y++;
        }
        if (__builtin_expect(final_var < 1, 0)) {
            z++;
        }
    }
    
    global_counter1 += x + y + z;
}
