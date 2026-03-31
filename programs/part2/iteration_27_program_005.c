/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int phi_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back */
        int a = x;
        int b = a;
        int c = b;
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {
            result += 1;
            global_array[i & 255] += 1;
        }
        
        /* Another comparison with 1 */
        if (c == 1) {
            result -= 1;
            global_counter++;
        }
        
        x_prev = x;
    }
    return result;
}

__attribute__((noinline, noipa))
int phi_pattern_b(int iterations, int seed) {
    /* Pattern B: Merge point phi from conditional assignment */
    int result = 0;
    volatile int vseed = seed; /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi at merge point */
        int cond = (vseed + i) & 1;
        int val;
        
        if (cond) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Chain assignments */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Compare phi result against 1 */
        if (tmp2 == 1) {
            result += i;
            global_array[(i + 1) & 255] ^= val;
        }
        
        /* Also compare against 0 */
        if (tmp2 == 0) {
            result -= i;
            global_counter--;
        }
    }
    return result;
}

__attribute__((noinline, noipa))
int phi_pattern_c(int iterations) {
    /* Pattern C: Complex phi network with multiple predecessors */
    int result = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; ++i) {
        int next_state;
        
        /* Create phi with multiple incoming values */
        switch (state) {
            case 0:
                next_state = (i % 3 == 0) ? 1 : 2;
                break;
            case 1:
                next_state = (i % 5 == 0) ? 0 : 2;
                break;
            case 2:
                next_state = (i % 7 == 0) ? 0 : 1;
                break;
            default:
                next_state = 0;
        }
        
        /* Multiple assignment chain */
        int s1 = next_state;
        int s2 = s1;
        int s3 = s2;
        
        /* Multiple comparisons with 0 and 1 */
        if (s3 == 0) {
            result += 100;
            global_array[i & 255] = i;
        }
        
        if (s3 == 1) {
            result += 200;
            global_counter += 2;
        }
        
        state = next_state;
    }
    return result;
}

__attribute__((noinline, noipa))
int nested_phi_pattern(int iterations, int mod) {
    /* Nested control flow creating complex phi nodes */
    int result = 0;
    int x = 0;
    int y = 1;
    
    for (int i = 0; i < iterations; ++i) {
        int inner_result = 0;
        
        /* Inner loop creates additional phi nodes */
        for (int j = 0; j < 10; ++j) {
            /* Phi from loop carried dependency */
            int z = (j == 0) ? x : y;
            
            /* Assignment chain */
            int z1 = z;
            int z2 = z1;
            
            /* Comparison with 0 */
            if (z2 == 0) {
                inner_result += j;
            }
            
            /* Update for next iteration */
            y = x;
            x = (i + j) % mod;
        }
        
        result += inner_result;
        
        /* Outer comparison with 1 */
        int outer_val = (result > 0) ? 1 : 0;
        int ov1 = outer_val;
        int ov2 = ov1;
        
        if (ov2 == 1) {
            global_counter++;
            global_array[i & 255] = result;
        }
    }
    return result;
}

__attribute__((noinline, noipa))
int hot_function(int iterations) {
    /* Main hot function combining all patterns */
    int total = 0;
    
    /* Mix different patterns to create varied control flow */
    total += phi_pattern_a(iterations / 4, 0);
    total += phi_pattern_b(iterations / 4, 42);
    total += phi_pattern_c(iterations / 4);
    total += nested_phi_pattern(iterations / 4, 17);
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default - should be hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Reset globals */
    global_counter = 0;
    memset(global_array, 0, sizeof(global_array));
    
    /* Execute hot function many times */
    int result = 0;
    for (int i = 0; i < 10; ++i) {
        result += hot_function(iterations);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = result;
    for (int i = 0; i < 256; ++i) {
        checksum ^= global_array[i];
    }
    checksum += global_counter;
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
