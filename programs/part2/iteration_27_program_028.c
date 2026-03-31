/* test_auto_profile.c - AutoFDO coverage test for phi-driven conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256] = {0};

/* Function attributes to prevent inlining and preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi feeding comparison with 0/1 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 - this is the target condition */
        if (c == 0) {
            /* Hot path - executed many times */
            global_array[i & 255] += 1;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            global_array[i & 255] -= 1;
        }
        
        /* Another comparison against constant 1 */
        if (x == 1) {
            global_counter += 2;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_b(int iterations) {
    /* Pattern B: Merge point phi feeding comparison */
    volatile int seed = iterations; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point */
        int val;
        if (seed & 1) {
            val = 1;  /* One arm of phi */
        } else {
            val = 0;  /* Other arm of phi */
        }
        
        /* Multiple assignments to obscure origin */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against constant 1 */
        if (tmp2 == 1) {
            global_array[(i + 1) & 255] ^= val;
            global_counter += 3;
        }
        
        /* Force branch unpredictability */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_c(int iterations) {
    /* Pattern C: Complex phi network with boolean comparisons */
    bool flag1 = false;
    bool flag2 = true;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi for boolean values */
        bool current_flag;
        if (i & 1) {
            current_flag = flag1;
        } else {
            current_flag = flag2;
        }
        
        /* Boolean comparison becomes (phi_var == 1) or (phi_var == 0) */
        if (current_flag) {  /* Implicit comparison with 1 */
            global_counter += 5;
        }
        
        /* Explicit comparison with 0 */
        if (current_flag == 0) {
            global_array[i & 255] |= 0x01;
        }
        
        /* Toggle flags to create varying phi inputs */
        flag1 = !flag1;
        flag2 = (i % 3) == 0;
    }
}

__attribute__((noinline, noipa))
void hot_function_mixed(int iterations) {
    /* Mixed pattern with multiple phi nodes and comparisons */
    int state = 0;
    char char_state = 'A';
    
    for (int i = 0; i < iterations; ++i) {
        /* Phi for integer state */
        int next_state;
        if (state > 100) {
            next_state = 0;
        } else {
            next_state = state + (i & 3);
        }
        
        /* Phi for character state */
        char next_char;
        if (char_state == 'Z') {
            next_char = 'A';
        } else {
            next_char = char_state + 1;
        }
        
        /* Chain assignments for integer state */
        int s1 = next_state;
        int s2 = s1;
        int s3 = s2;
        
        /* Comparisons against 0 and 1 */
        if (s3 == 0) {
            global_counter += 7;
            global_array[0] = next_char;
        }
        
        if ((s3 & 1) == 1) {  /* Comparison with 1 */
            global_array[1] = next_char;
        }
        
        /* Update states for next iteration */
        state = next_state;
        char_state = next_char;
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
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute all patterns to generate diverse profile data */
    hot_function_pattern_a(iterations / 4, 0);
    hot_function_pattern_b(iterations / 4);
    hot_function_pattern_c(iterations / 4);
    hot_function_mixed(iterations / 4);
    
    /* Additional runs with different parameters to create varied phi inputs */
    for (int run = 0; run < 10; ++run) {
        hot_function_pattern_a(1000, run);
        hot_function_pattern_b(1000);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;  /* Return non-zero if everything was optimized away */
}
