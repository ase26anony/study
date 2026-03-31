/* test_auto_profile.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
int results[1000] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int phi_defined_variable(int iteration, int start_val) {
    int phi_val;
    
    /* Pattern B: Merge point phi - val becomes a phi node at merge point */
    if (iteration & 1) {
        phi_val = 1;  /* Taken path */
    } else {
        phi_val = 0;  /* Not taken path */
    }
    
    /* Pattern C: Chained copies to obscure origin */
    int copy1 = phi_val;
    int copy2 = copy1;
    int final_val = copy2;
    
    /* Critical comparison against constant 0 or 1 */
    if (final_val == 0) {  /* Line we want to cover - comparison with 0 */
        return 1;
    } else {
        return 0;
    }
}

__attribute__((noinline, noipa))
int loop_dependent_phi(int n) {
    int prev = 0;
    int sum = 0;
    
    /* Pattern A: Loop-dependent condition with phi */
    for (int i = 0; i < n; ++i) {
        /* x is a phi node merging values from loop header and previous iteration */
        int x;
        if (i == 0) {
            x = 1;  /* Initial value */
        } else {
            x = prev + (i & 1);  /* Phi merges two values */
        }
        
        /* Chain assignments */
        int y = x;
        int z = y;
        
        /* Comparison against constant 1 */
        if (z == 1) {  /* Line we want to cover - comparison with 1 */
            sum += i;
            global_counter++;  /* Side effect */
        }
        
        prev = x;
    }
    return sum;
}

__attribute__((noinline, noipa))
int complex_phi_pattern(int seed, int iterations) {
    int state = seed;
    int total = 0;
    
    /* Create multiple phi nodes in nested control flow */
    for (int i = 0; i < iterations; ++i) {
        int a, b;
        
        /* First level of phi nodes */
        if (state & 1) {
            a = 1;
            b = 0;
        } else {
            a = 0;
            b = 1;
        }
        
        /* Second level phi - depends on first level phis */
        int combined;
        if (a > b) {
            combined = a;
        } else {
            combined = b;
        }
        
        /* Multiple assignment chains */
        int chain1 = combined;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Mix of comparisons against 0 and 1 */
        if (chain3 == 0) {
            total += i;
        }
        if (chain2 == 1) {
            total -= i;
        }
        
        /* Update state with side effects */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        results[i % 1000] = total;
    }
    
    return total;
}

/* Hot function that will be called many times */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    int sum = 0;
    
    /* Call different patterns to ensure coverage */
    for (int i = 0; i < iterations; ++i) {
        /* Mix patterns to create various phi scenarios */
        sum += phi_defined_variable(i, i % 2);
        sum += loop_dependent_phi(10);
        
        /* Use volatile to prevent constant folding */
        volatile int r = rand() % 100;
        if (r < 50) {
            sum += complex_phi_pattern(i, 5);
        }
        
        /* Ensure side effects */
        global_counter += (sum & 1);
    }
    
    /* Store result to prevent dead code elimination */
    results[0] = sum;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Clear results array */
    memset(results, 0, sizeof(results));
    
    /* Execute hot function many times to create profile */
    hot_function(iterations);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 1000; ++i) {
        checksum ^= results[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
