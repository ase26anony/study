/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-to-conditional analysis */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization and create side effects */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_pattern_a(int iterations) {
    /* Pattern A: Loop-dependent phi feeding comparison with 0/1 */
    int prev_x = 0;
    int x = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; ++i) {
        /* x is a phi merging from loop header (prev_x) and previous iteration */
        x = (i == 0) ? 0 : prev_x + (i % 3);
        
        /* Chain of assignments to test the while loop walking back */
        int temp1 = x;
        int temp2 = temp1;
        int temp3 = temp2;
        
        /* Comparison against 0 - this should trigger the uncovered code */
        if (temp3 == 0) {
            global_array[i % 256] += 1;
            global_counter++;
        }
        
        /* Another comparison against 1 */
        if (temp3 == 1) {
            global_array[(i + 128) % 256] += 2;
            global_counter += 2;
        }
        
        prev_x = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_b(int iterations) {
    /* Pattern B: Merge point phi from conditional assignment */
    volatile int seed = iterations; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point */
        int val;
        if ((seed + i) & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Multiple assignments to obscure origin */
        int a = val;
        int b = a;
        
        /* Comparison against constant 1 */
        if (b == 1) {
            global_counter += 3;
            global_array[i % 256] *= 2;
        }
        
        /* Another path with comparison against 0 */
        if (b == 0) {
            global_counter += 5;
            global_array[(i + 64) % 256] /= 2;
        }
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_c(int iterations) {
    /* Pattern C: Complex phi network with boolean type */
    bool flag1 = false;
    bool flag2 = true;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create multiple phis that interact */
        bool cond1 = (i % 7) == 0;
        bool cond2 = (i % 11) == 0;
        
        /* Phi merging two boolean values */
        bool merged_flag;
        if (cond1) {
            merged_flag = flag1;
        } else {
            merged_flag = flag2;
        }
        
        /* Chain of assignments with different types */
        char c1 = merged_flag;
        short s1 = c1;
        int i1 = s1;
        
        /* Boolean comparisons (will be with 0/1) */
        if (i1 == 1) {  /* true comparison */
            global_counter += 7;
            global_array[i % 256] |= 0xFF;
        }
        
        /* Inverted comparison */
        if (i1 == 0) {  /* false comparison */
            global_counter += 11;
            global_array[(i + 192) % 256] &= 0x0F;
        }
        
        /* Update phi sources for next iteration */
        flag1 = !flag1;
        flag2 = (i % 3) == 0;
    }
}

__attribute__((noinline, noipa))
void mixed_patterns(int iterations) {
    /* Mix all patterns to increase coverage probability */
    for (int phase = 0; phase < 3; ++phase) {
        switch (phase) {
            case 0:
                hot_function_pattern_a(iterations / 3);
                break;
            case 1:
                hot_function_pattern_b(iterations / 3);
                break;
            case 2:
                hot_function_pattern_c(iterations / 3);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default - should create hot paths */
    
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
    
    /* Execute hot functions multiple times to ensure profiling */
    for (int run = 0; run < 10; ++run) {
        mixed_patterns(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
