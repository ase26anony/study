/* test_auto_profile.c - Test program for AutoFDO phi-node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    int x_prev = start_val;
    
    /* Pattern A: Loop-dependent phi node feeding condition */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Insert chained assignments (Pattern C) */
        int a = x;
        int b = a;
        int c = b;
        
        /* Compare against constant 0 (uncovered line requirement) */
        if (c == 0) {
            global_array[i & 255] += 1;
            global_counter++;
        }
        
        /* Also compare against constant 1 */
        if (c == 1) {
            global_array[(i + 128) & 255] += 2;
            global_counter += 2;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int iterations, int seed) {
    volatile int external = seed; /* Prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern B: Merge point phi from conditional assignment */
        int val = (external & 1) ? 1 : 0;
        
        /* Chain assignments to test the while loop walking back */
        int tmp1 = val;
        int tmp2 = tmp1;
        int tmp3 = tmp2;
        
        /* Compare phi-derived value against constant 1 */
        if (tmp3 == 1) {
            global_array[i & 255] += 3;
            global_counter += 3;
        }
        
        /* Force external to change to create varying conditions */
        external = (external * 1103515245 + 12345) & 0x7fffffff;
    }
}

__attribute__((noinline, noipa))
void hot_function_bool_phi(int iterations) {
    bool flag = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi node for boolean */
        flag = !flag;
        
        /* Chain through different types (Pattern C variant) */
        char c = flag;
        short s = c;
        int tmp = s;
        
        /* Boolean comparison against 0/1 */
        if (tmp == 0) {
            global_array[(i * 7) & 255] += 5;
            global_counter += 5;
        }
        
        if (tmp == 1) {
            global_array[(i * 13) & 255] += 7;
            global_counter += 7;
        }
    }
}

__attribute__((noinline, noipa))
void hot_function_nested_phi(int iterations, int mod) {
    int a = 0, b = 1;
    
    for (int i = 0; i < iterations; ++i) {
        /* Complex phi network */
        int selector = (i % mod) == 0 ? 1 : 0;
        int val;
        
        if (selector == 0) {
            val = a;
            a = (a + 1) & 1;
        } else {
            val = b;
            b = (b + 1) & 1;
        }
        
        /* Multiple chained assignments */
        int x1 = val;
        int x2 = x1;
        int x3 = x2;
        
        /* Multiple comparisons to increase coverage */
        if (x3 == 0) {
            global_counter += 11;
        }
        
        if (x3 == 1) {
            global_counter += 13;
        }
        
        /* Also test != which may invert to == */
        if (x3 != 0) {
            global_array[i & 255] += 17;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000; /* Default - make it hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Reset globals */
    global_counter = 0;
    for (int i = 0; i < 256; i++) global_array[i] = 0;
    
    /* Execute all patterns to ensure coverage */
    hot_function_phi_loop(iterations / 4, 0);
    hot_function_merge_phi(iterations / 4, 42);
    hot_function_bool_phi(iterations / 4);
    hot_function_nested_phi(iterations / 4, 7);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 256; i++) {
        checksum = (checksum * 31 + global_array[i]) & 0xffff;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
