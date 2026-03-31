/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based conditional branches */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1000] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int hot_function_phi_loop(int iterations, int start_val) {
    int result = 0;
    int prev_x = 0;
    
    /* Pattern A: Loop-dependent phi node feeding comparison */
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : prev_x + 1 */
        int x;
        if (i == 0) {
            x = start_val;
        } else {
            x = prev_x + 1;
        }
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        /* Pattern C: Chained copies */
        int a = x;      /* First copy */
        int b = a;      /* Second copy */
        int c = b;      /* Third copy - final comparison uses this */
        
        /* Comparison against constant 0 */
        if (c == 0) {   /* This should trigger the uncovered code */
            result += 1;
            global_array[i % 1000] = i;
        }
        
        /* Comparison against constant 1 */
        if (c == 1) {   /* Another comparison against constant 1 */
            result += 2;
            global_counter++;
        }
        
        prev_x = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int hot_function_merge_phi(int flag) {
    /* Pattern B: Merge point phi */
    int val;
    
    /* Create a phi node at merge point */
    if (flag) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Chain assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Comparison against constant 1 */
    if (tmp2 == 1) {   /* Should trigger uncovered code */
        global_array[flag % 1000] += flag;
        return 1;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int hot_function_bool_phi(bool cond1, bool cond2) {
    /* Boolean phi node that compares against 0/1 */
    bool flag;
    
    /* Create phi from two conditions */
    if (cond1) {
        flag = true;    /* Becomes 1 */
    } else {
        flag = false;   /* Becomes 0 */
    }
    
    /* Additional complexity with nested conditions */
    bool final_flag;
    if (cond2) {
        final_flag = flag;
    } else {
        final_flag = !flag;
    }
    
    /* Comparison against boolean constant (becomes 0/1) */
    if (final_flag == true) {   /* Should be optimized to == 1 */
        return 100;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
void complex_phi_pattern(int n) {
    /* More complex pattern with multiple phi nodes */
    int a = 0, b = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple phi nodes in loop */
        int x, y;
        
        if (i == 0) {
            x = 0;
            y = 1;
        } else {
            x = a + 1;
            y = b * 2;
        }
        
        /* Chain assignments */
        int x1 = x;
        int x2 = x1;
        int x3 = x2;
        
        int y1 = y;
        int y2 = y1;
        
        /* Multiple comparisons against 0/1 */
        if (x3 == 0) {
            global_counter += i;
        }
        
        if (y2 == 1) {
            global_array[i % 1000] = y2;
        }
        
        /* Mix with volatile to prevent optimization */
        volatile int v = i;
        if (v % 3 == 0) {
            x3 = 0;  /* Force some paths to take comparison */
        }
        
        if (x3 == 0) {  /* Another comparison */
            global_counter--;
        }
        
        a = x;
        b = y;
    }
}

/* Helper to ensure code isn't dead */
__attribute__((noinline))
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += global_array[i];
        sum += i;
    }
    return sum + global_counter;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Execute hot functions to generate profile data */
    int total = 0;
    
    /* Pattern A with loop phi */
    total += hot_function_phi_loop(iterations, 0);
    total += hot_function_phi_loop(iterations / 2, 1);
    
    /* Pattern B with merge phi */
    for (int i = 0; i < iterations / 10; i++) {
        total += hot_function_merge_phi(i & 1);
    }
    
    /* Pattern with boolean phi */
    for (int i = 0; i < iterations / 5; i++) {
        total += hot_function_bool_phi((i & 2) != 0, (i & 4) != 0);
    }
    
    /* Complex pattern */
    complex_phi_pattern(iterations / 3);
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = compute_checksum();
    printf("Result: %d, Checksum: %d\n", total, checksum);
    
    return 0;
}
