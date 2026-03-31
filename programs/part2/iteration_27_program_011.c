/* test_auto_profile.c - Test program for AutoFDO phi-node branch coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0 */
    int x_prev = start_val;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Critical comparison: phi-derived variable compared to 0 */
        if (c == 0) {   /* Should trigger: cmp_rhs is constant 0 */
            global_array[i & 255] += 1;
            result += 1;
        }
        
        /* Another comparison with 1 */
        int d = (x % 2 == 0) ? 1 : 0;  /* Another phi node */
        int e = d;                     /* Chain assignment */
        if (e == 1) {                  /* cmp_rhs is constant 1 */
            global_counter++;
        }
        
        x_prev = x;
    }
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        printf(".");  /* Side effect */
    }
}

/* Pattern B: Merge point phi */
__attribute__((noinline, noipa))
void hot_function_merge(int flag) {
    /* Create phi at merge point */
    int val;
    if (flag) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Chain assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Comparison with 1 */
    if (tmp2 == 1) {
        global_array[flag & 255] += 100;
    }
    
    /* Another comparison with 0 */
    int tmp3 = (val == 1) ? 0 : 1;  /* Another phi */
    if (tmp3 == 0) {
        global_counter += 2;
    }
}

/* Pattern C: Complex phi network with boolean */
__attribute__((noinline, noipa))
void hot_function_bool(int a, int b) {
    /* Create boolean phi */
    _Bool cond1 = (a > b);
    _Bool cond2 = (a < b * 2);
    
    /* Phi from two boolean values */
    _Bool final_cond = cond1 && cond2;
    
    /* Chain through different types to create distinct SSA names */
    char c1 = final_cond;
    short s1 = c1;
    int i1 = s1;
    
    /* Comparison - bool with true/false becomes 1/0 */
    if (i1 == 1) {  /* Should use constant 1 */
        global_array[a & 255] += a;
    }
    
    /* Inverted comparison */
    if (i1 == 0) {  /* Should use constant 0 */
        global_array[b & 255] += b;
    }
}

/* Main driver that creates hot paths */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - creates hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    printf("Running %d iterations...\n", iterations);
    
    /* Create hot path for Pattern A */
    for (int i = 0; i < iterations; i++) {
        /* Vary start_val to prevent constant folding */
        int start_val = (i % 3 == 0) ? 0 : 1;
        hot_function_phi_loop(100, start_val);
        
        /* Also call other patterns to ensure coverage */
        if (i % 100 == 0) {
            hot_function_merge(i & 1);
            hot_function_bool(i, i * 2);
        }
    }
    
    /* Calculate checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("\nChecksum: %d\n", checksum);
    
    return 0;
}
