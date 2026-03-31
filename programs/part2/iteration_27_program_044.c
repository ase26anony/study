/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-node branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Function to create side effects and prevent dead code elimination */
__attribute__((noinline)) void side_effect(int idx, int val) {
    global_array[idx & 1023] ^= val;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
__attribute__((noinline, noipa)) 
void pattern_a_loop_phi(int iterations) {
    int prev_x = 0;
    int temp1, temp2, temp3;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi node: x is defined differently on first iteration vs others */
        int x;
        if (i == 0) {
            x = 1;  /* Initial value */
        } else {
            x = prev_x + (global_counter & 1);  /* Phi merges previous value with computation */
        }
        
        /* Chain of assignments to test the while loop walking back through GIMPLE_ASSIGN */
        temp1 = x;          /* First copy */
        temp2 = temp1;      /* Second copy */
        temp3 = temp2;      /* Third copy */
        
        /* Critical comparison: phi-derived value compared to 0 */
        if (temp3 == 0) {   /* This should trigger the uncovered code */
            side_effect(i, 1);
        } else {
            side_effect(i, 2);
        }
        
        /* Also test comparison with 1 */
        if (x == 1) {       /* Direct phi comparison with 1 */
            side_effect(i + 512, 3);
        }
        
        prev_x = x;
    }
}

/* Pattern B: Conditional merge phi */
__attribute__((noinline, noipa))
void pattern_b_merge_phi(int iterations) {
    volatile int seed = global_counter;  /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        int cond = (seed + i) & 1;  /* Varies between 0 and 1 */
        
        /* Create phi at merge point */
        int phi_val;
        if (cond) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Multiple assignments to obscure origin */
        int a = phi_val;
        int b = a;
        
        /* Comparison with constant 0 */
        if (b == 0) {  /* Should trigger uncovered code */
            side_effect(i * 2, 4);
        }
        
        /* Another comparison with constant 1 */
        if (phi_val == 1) {  /* Direct comparison */
            side_effect(i * 3, 5);
        }
    }
}

/* Pattern C: Complex phi network with boolean type */
__attribute__((noinline, noipa))
void pattern_c_bool_phi(int iterations) {
    bool flag1 = false;
    bool flag2 = true;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create multiple phi nodes */
        bool phi1;
        if (i & 1) {
            phi1 = flag1;
        } else {
            phi1 = flag2;
        }
        
        bool phi2;
        if (i & 2) {
            phi2 = phi1;
        } else {
            phi2 = !phi1;
        }
        
        /* Chain assignments with different types */
        int as_int = (int)phi2;
        short as_short = (short)as_int;
        char as_char = (char)as_short;
        int final_val = (int)as_char;
        
        /* Comparisons that should map to == 0 or == 1 */
        if (final_val == 0) {  /* Should trigger uncovered code */
            side_effect(i, 6);
        }
        
        if (phi2 == true) {  /* Boolean comparison with true (== 1) */
            side_effect(i + 256, 7);
        }
        
        /* Toggle flags to create varying phi values */
        flag1 = !flag1;
        flag2 = (i % 3) == 0;
    }
}

/* Main hot function that calls all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    /* Mix patterns to create diverse control flow */
    pattern_a_loop_phi(iterations / 3);
    pattern_b_merge_phi(iterations / 3);
    pattern_c_bool_phi(iterations / 3);
    
    /* Additional mixed pattern in same function */
    int x = 0;
    for (int i = 0; i < iterations / 10; ++i) {
        /* Another phi pattern */
        int y = (x == 0) ? 1 : 0;  /* Phi node */
        int z = y;
        
        if (z == 0) {  /* Comparison with 0 */
            side_effect(i, 8);
        }
        
        x = (x + 1) & 1;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Run hot function multiple times to ensure basic blocks get annotated */
    for (int run = 0; run < 3; ++run) {
        hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}
