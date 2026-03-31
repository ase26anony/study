/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
static int phi_merge_pattern(int cond, int val1, int val2) {
    /* Pattern B: Merge point phi */
    int merged = cond ? val1 : val2;  /* This creates a phi node */
    
    /* Chain assignments to test the while loop walking back */
    int a = merged;    /* GIMPLE_ASSIGN copy */
    int b = a;         /* Another copy */
    int c = b;         /* Final copy before comparison */
    
    /* Compare against constant 0 */
    if (c == 0) {      /* This should trigger the uncovered code */
        global_array[0] += 1;
        return 1;
    }
    return 0;
}

__attribute__((noinline, noipa))
static int loop_phi_pattern(int iterations) {
    /* Pattern A: Loop-dependent phi */
    int prev = 0;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* 'x' is a phi node: φ(prev, i==0?0:prev+1) */
        int x = (i == 0) ? 0 : prev + 1;
        
        /* Chain assignments */
        int y = x;
        int z = y;
        
        /* Compare against constant 1 */
        if (z == 1) {  /* This should also trigger the uncovered code */
            result += global_array[i & 255];
        }
        
        /* Mix in another comparison against 0 */
        if (x == 0) {
            global_counter++;
        }
        
        prev = x;
    }
    return result;
}

__attribute__((noinline, noipa))
static int complex_phi_chain(int seed) {
    /* Pattern C: Complex phi with multiple chains */
    int base = seed & 1;
    
    /* Create multiple conditional merges */
    int val1 = (seed & 2) ? 1 : 0;
    int val2 = (seed & 4) ? base : val1;
    int val3 = (seed & 8) ? val2 : 0;
    
    /* This creates a phi at the ?: operator */
    int phi_val = (global_counter > 1000) ? val3 : base;
    
    /* Multiple assignment chains */
    int chain1 = phi_val;
    int chain2 = chain1;
    int chain3 = chain2;
    int chain4 = chain3;
    
    /* Both comparisons against 0 and 1 */
    if (chain4 == 0) {
        return 0;
    } else if (chain4 == 1) {
        return 1;
    }
    return -1;
}

__attribute__((noinline, noipa))
static void hot_function(int iterations) {
    int sum = 0;
    
    /* Execute all patterns multiple times to ensure hot paths */
    for (int i = 0; i < iterations; ++i) {
        /* Use volatile to prevent constant folding */
        volatile int r = rand() & 0xF;
        
        /* Pattern B with merge phi */
        sum += phi_merge_pattern(r & 1, 0, 1);
        
        /* Pattern A with loop phi (small inner loop) */
        if (i % 3 == 0) {
            sum += loop_phi_pattern(10);
        }
        
        /* Pattern C with complex chains */
        if (i % 5 == 0) {
            sum += complex_phi_chain(r);
        }
        
        /* Direct boolean phi comparison */
        _Bool flag = (i % 7 == 0);
        _Bool flag2 = flag;
        if (flag2 == 1) {  /* Comparison with boolean true (== 1) */
            global_array[i & 255] = i;
        }
    }
    
    /* Use result to prevent dead code elimination */
    global_counter += sum;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large iteration count */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Main hot loop - this will be profiled as hot */
    printf("Starting %d iterations...\n", iterations);
    
    for (int outer = 0; outer < 10; ++outer) {
        hot_function(iterations / 10);
    }
    
    /* Compute checksum to ensure all code executed */
    int checksum = global_counter;
    for (int i = 0; i < 256; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
