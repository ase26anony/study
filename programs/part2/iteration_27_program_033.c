/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-node based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int hot_function_phi_loop(int iterations, int start_val) {
    int result = 0;
    int prev_x = start_val;
    
    /* Pattern A: Loop-dependent phi node feeding comparison with 0/1 */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : prev_x + 1 */
        int x = (i == 0) ? start_val : prev_x + 1;
        
        /* Pattern C: Chain of assignments to test the while loop walking back */
        int a = x;
        int b = a;
        int c = b;
        
        /* Comparison against constant 0 - this should trigger the uncovered code */
        if (c == 0) {
            /* Hot path - executed many times */
            result += 1;
            global_array[i & 255] = i;
        } else if (c == 1) {
            /* Also test comparison against 1 */
            result += 2;
            global_counter++;
        }
        
        prev_x = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_merge_phi(int flag, int val1, int val2) {
    /* Pattern B: Merge point phi from conditional assignment */
    int merged_val;
    
    /* This creates a phi node at the merge point */
    if (flag) {
        merged_val = val1;
    } else {
        merged_val = val2;
    }
    
    /* Chain assignments to obscure origin */
    int tmp1 = merged_val;
    int tmp2 = tmp1;
    int final_val = tmp2;
    
    /* Comparison against 1 */
    if (final_val == 1) {
        global_counter += 10;
        return 100;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int hot_function_bool_phi(int a, int b) {
    /* Boolean phi node that compares against 0/1 */
    bool condition = (a > b);
    
    /* Multiple assignments to create SSA chain */
    bool cond1 = condition;
    bool cond2 = cond1;
    bool final_cond = cond2;
    
    /* This generates comparison against 0 or 1 */
    if (final_cond) {  /* Implicitly: final_cond != 0 */
        return a + b;
    }
    
    return a - b;
}

__attribute__((noinline, noipa))
void complex_phi_pattern(int n) {
    int x = 0;
    int y = 1;
    
    /* Create multiple phi nodes in nested loops */
    for (int i = 0; i < n; i++) {
        /* Phi for x based on loop iteration */
        int phi_x = (i == 0) ? 0 : x + 1;
        
        for (int j = 0; j < 10; j++) {
            /* Another phi inside nested loop */
            int phi_y = (j == 0) ? phi_x : y;
            
            /* Chain assignments */
            int val1 = phi_y;
            int val2 = val1;
            
            /* Multiple comparisons against 0 and 1 */
            if (val2 == 0) {
                global_array[(i + j) & 255] += 1;
            }
            
            if (val2 == 1) {
                global_counter++;
            }
            
            y = phi_y;
        }
        
        x = phi_x;
    }
}

/* Separate compilation unit simulation */
__attribute__((noinline, noipa))
int external_helper(int x) {
    return x * 2 + 1;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    int total = 0;
    
    /* Execute hot functions many times to create profile */
    for (int run = 0; run < 10; run++) {
        /* Pattern A with loop phi */
        total += hot_function_phi_loop(iterations / 10, run % 3);
        
        /* Pattern B with merge phi */
        total += hot_function_merge_phi(run & 1, 1, 0);
        
        /* Boolean phi pattern */
        total += hot_function_bool_phi(run, run * 2);
        
        /* Complex pattern */
        complex_phi_pattern(iterations / 100);
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int checksum = total + global_counter;
    
    /* Compute checksum to ensure all code has effect */
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
