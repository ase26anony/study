/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-to-conditional branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to maintain SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0/1 */
    int phi_val = start_val;
    int prev_val = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a phi node: phi_val merges values from start_val and prev_val */
        int temp = (i == 0) ? start_val : prev_val + (rand() % 3 - 1); /* -1, 0, or 1 */
        
        /* Pattern C: Chain assignments to test the while loop walking back */
        int a = temp;
        int b = a;
        int c = b;
        
        /* Critical comparison: phi-derived variable vs constant 0 */
        if (c == 0) {
            global_array[i % 256] += 1;
            global_counter++;
        }
        
        /* Another comparison with constant 1 */
        int d = (c > 0) ? 1 : 0;  /* Creates another phi */
        int e = d;
        if (e == 1) {
            global_array[(i + 128) % 256] -= 1;
        }
        
        prev_val = temp;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int flag1, int flag2) {
    /* Pattern B: Merge point phi from conditional assignment */
    int merged_val;
    
    if (flag1) {
        merged_val = 1;
    } else {
        merged_val = 0;
    }
    
    /* Chain assignments */
    int x = merged_val;
    int y = x;
    
    /* Comparison with constant 1 */
    if (y == 1) {
        global_counter += 2;
        global_array[0] = 1;
    }
    
    /* Nested condition creating complex phi network */
    int final_val;
    if (flag2) {
        final_val = merged_val;
    } else {
        final_val = (flag1) ? 0 : 1;
    }
    
    /* Another comparison with constant 0 */
    if (final_val == 0) {
        global_array[1] = -1;
    }
}

__attribute__((noinline, noipa))
void hot_function_bool_phi(int mode) {
    /* Boolean phi nodes that compare with true/false (which become 1/0) */
    bool condition1 = (mode & 1) != 0;
    bool condition2 = (mode & 2) != 0;
    
    /* Phi from two boolean sources */
    bool combined = condition1 ? condition2 : false;
    
    /* Chain through assignments */
    bool b1 = combined;
    bool b2 = b1;
    
    /* Comparison with boolean constant (becomes == 1) */
    if (b2 == true) {
        global_array[2] = mode;
    }
    
    /* Another boolean phi with comparison to false (becomes == 0) */
    bool alt_combined = condition1 ? true : condition2;
    if (alt_combined == false) {
        global_counter += 3;
    }
}

__attribute__((noinline, noipa))
void hot_function_mixed_types(char init_char, short init_short) {
    /* Mix types to create different SSA names */
    char c = init_char;
    short s = init_short;
    
    /* Phi with different types */
    int mixed = (c == 'A') ? s : (int)c;
    
    /* Chain assignments */
    int m1 = mixed;
    int m2 = m1;
    
    /* Comparison with 0 */
    if (m2 == 0) {
        global_array[3] = c;
    }
    
    /* Another phi from type conversion */
    int converted = (s > 100) ? 1 : 0;
    if (converted == 1) {
        global_counter += s;
    }
}

/* Main driver that creates hot paths */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - make paths hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Seed random for variability but reproducible hot paths */
    srand(42);
    
    /* Execute hot functions many times to create profile annotations */
    for (int outer = 0; outer < 10; ++outer) {
        /* Pattern A: Loop phi with comparisons to 0 and 1 */
        hot_function_phi_loop(iterations / 10, outer % 3);
        
        /* Pattern B: Merge point phi */
        hot_function_merge_phi(outer & 1, outer & 2);
        
        /* Boolean phi patterns */
        hot_function_bool_phi(outer);
        
        /* Mixed type phi patterns */
        hot_function_mixed_types('A' + (outer % 26), outer * 10);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
