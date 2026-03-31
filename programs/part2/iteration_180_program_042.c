/* Program to trigger delay slot filling in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int g_volatile = 0;

/* Function with complex enough control flow to require scheduling */
int process_values(int argc, char **argv) {
    /* Declare and initialize variables - use different registers */
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int mod_base = (argc > 2) ? 7 : 11;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Mix of operations to create register pressure */
        e = f + g;
        f = g ^ h;
        g = h << 2;
        h = e >> 1;
        
        /* Key construct: conditional jump with potential delay slot candidate */
        /* Use runtime-dependent condition to prevent constant folding */
        if ((i % mod_base) == (g_volatile & 3)) {
            /* Jump to label where candidate instruction resides */
            goto target_label;
        }
        
        /* Alternative path with different operations */
        a = b + c;
        b = c * d;
        c = d | a;
        d = a & b;
        
        /* Continue after the label */
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* Candidate for delay slot: simple arithmetic, no trapping */
        /* Uses variables not in jump condition, not live across jump */
        a = b + c;  /* Simple add - safe, non-trapping */
        
        /* Additional operations to ensure not a single-instruction block */
        b = c * d;
        c = d | a;
    }
    
    /* Post-loop computations to create observable side effects */
    result = a + b + c + d + e + f + g + h;
    
    /* Use result to prevent dead code elimination */
    if (g_volatile) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Another function to increase compilation complexity */
int helper_func(int x, int y) {
    int r = 0;
    for (int i = 0; i < 10; ++i) {
        if (x & (1 << i)) {
            r += y * i;
        } else {
            r -= y / (i + 1);
        }
    }
    return r;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Call processing function multiple times */
    for (int j = 0; j < 3; ++j) {
        total += process_values(argc + j, argv);
    }
    
    /* Mix with other function to create more scheduling opportunities */
    total += helper_func(argc, total);
    
    /* Final result based on input to prevent optimization */
    return (total > 0) ? 0 : 1;
}
