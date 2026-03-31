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
        /* Create two SSA variables with trivial operations */
        int a = base_value + i;      /* SSA variable a */
        int b = base_value - i;      /* SSA variable b */
        
        /* Create phi-node: conditionally assign a or b */
        int phi_var;
        if (i % 10 == 0) {           /* Creates phi-node in GIMPLE */
            phi_var = a;             /* PHI argument 1 */
        } else {
            phi_var = b;             /* PHI argument 2 */
        }
        
        /* Create chain of single SSA-to-SSA assignments */
        int tmp1 = phi_var;          /* First assignment in chain */
        int tmp2 = tmp1;             /* Second assignment in chain */
        int chain_var = tmp2;        /* Final variable for comparisons */
        
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
            result += 4;
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
    
    /* Add some complexity to prevent optimization */
    if (argc > 1) {
        base += atoi(argv[1]);
    }
    
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
    
    return total > 0 ? 0 : 1;
}

/* Additional function with different phi-node pattern */
__attribute__((hot))
static int another_hot_function(int start, int end) {
    int sum = 0;
    
    /* Loop with phi-node from two sources */
    for (int i = start; i < end; ++i) {
        int x = i * 2;
        int y = i / 2;
        
        /* Phi-node created by conditional operator */
        int phi_val = (i % 3 == 0) ? x : y;
        
        /* Chain of assignments */
        int intermediate = phi_val;
        int final_var = intermediate;
        
        /* Multiple comparisons with constants 0 and 1 */
        if (__builtin_expect(final_var == 0, 0)) {
            sum += 1;
        }
        
        if (__builtin_expect(final_var >= 1, 1)) {
            sum += 2;
        }
        
        if (__builtin_expect(final_var <= 1, 0)) {
            sum += 3;
        }
    }
    
    return sum;
}
