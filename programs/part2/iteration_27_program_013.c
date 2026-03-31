/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create side effects and prevent dead code elimination */
__attribute__((noinline, noipa))
void side_effect(int value) {
    global_array[value & 0xFF] += value;
    global_counter++;
}

/* Pattern A: Loop-dependent phi with chained assignments */
__attribute__((noinline, noipa))
void pattern_a_loop_phi(int iterations) {
    int prev = 0;
    int phi_val;
    
    for (int i = 0; i < iterations; i++) {
        /* This creates a phi node: phi_val = (i == 0) ? 0 : prev + 1 */
        if (i == 0) {
            phi_val = 0;
        } else {
            phi_val = prev + 1;
        }
        
        /* Chain of assignments to test the while loop walking back */
        int a = phi_val;
        int b = a;
        int c = b;
        int d = c;
        
        /* Comparison against constant 0 */
        if (d == 0) {
            side_effect(1);
        } else {
            side_effect(2);
        }
        
        /* Another comparison against constant 1 */
        int e = (d > 0) ? 1 : 0;  /* Creates another phi */
        if (e == 1) {
            side_effect(3);
        }
        
        prev = phi_val;
    }
}

/* Pattern B: Merge point phi from conditional assignment */
__attribute__((noinline, noipa))
int pattern_b_merge_phi(int x, int y) {
    int result;
    
    /* This creates a phi at the merge point */
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    /* Chain assignments */
    int tmp1 = result;
    int tmp2 = tmp1;
    
    /* Comparison against 1 */
    if (tmp2 == 1) {
        side_effect(x);
        return x;
    } else {
        side_effect(y);
        return y;
    }
}

/* Pattern C: Complex phi network with multiple predecessors */
__attribute__((noinline, noipa))
void pattern_c_complex_phi(int mode, int iterations) {
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        int next_state;
        
        /* Multiple conditionals creating complex phi */
        if (mode == 0) {
            next_state = (state + 1) % 3;
        } else if (mode == 1) {
            next_state = (state * 2) % 5;
        } else {
            next_state = state ^ 1;
        }
        
        /* Multiple chained copies */
        int v1 = next_state;
        int v2 = v1;
        int v3 = v2;
        
        /* Multiple comparisons against 0 and 1 */
        if (v3 == 0) {
            side_effect(100);
        }
        
        if (v3 == 1) {
            side_effect(200);
        }
        
        /* Boolean phi from comparison */
        bool is_special = (v3 == 2);
        bool b1 = is_special;
        bool b2 = b1;
        
        /* Comparison with bool (becomes 0/1) */
        if (b2 == 1) {
            side_effect(300);
        }
        
        state = next_state;
    }
}

/* Main hot function that combines all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations, int mode) {
    /* Mix different patterns to create various phi nodes */
    pattern_a_loop_phi(iterations / 3);
    
    for (int i = 0; i < iterations / 10; i++) {
        int x = rand() % 100;
        int y = rand() % 100;
        pattern_b_merge_phi(x, y);
    }
    
    pattern_c_complex_phi(mode, iterations / 4);
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    int mode = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1000) iterations = 1000000;
    }
    if (argc > 2) {
        mode = atoi(argv[2]) % 3;
    }
    
    /* Seed random for variability but reproducible with same input */
    srand(42);
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function many times to ensure basic blocks are annotated */
    for (int i = 0; i < 10; i++) {
        hot_function(iterations, mode);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    /* Output result to ensure code isn't optimized away */
    printf("Checksum: %d (counter: %d)\n", checksum, global_counter);
    
    return 0;
}
