/* Program to trigger delay slot filling in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int guard = 0;

/* Function containing the critical pattern */
int process_values(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed * 2;
    int d = seed - 5;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent optimization */
        int condition = (i + seed) & 0xF;
        
        /* 
         * CRITICAL PATTERN: Conditional jump to label with simple instruction at target
         * This should generate: if (condition == 0) goto target_label;
         */
        if (condition == 0) {
            goto target_label;
        }
        
        /* Some intermediate computation */
        a = b + c;
        b = c ^ d;
        
        /* Continue normal flow */
        continue;
        
    target_label:
        /* 
         * TARGET INSTRUCTION: Simple, safe operation for delay slot filling
         * Must not trap, not reference special resources, not be a jump
         * Simple register-to-register operation is ideal
         */
        c = a + d;  /* Simple addition - safe, non-trapping */
        
        /* Additional instruction after label to ensure it's not a single-instruction block */
        d = b | 0x1;
    }
    
    /* Use results to prevent dead code elimination */
    result = (a ^ b) + (c | d);
    
    /* Add some branching to keep the CFG interesting */
    if (guard) {
        result = -result;
    }
    
    return result;
}

/* Another function with similar pattern but different structure */
int alternate_pattern(int limit, int base) {
    int x = base;
    int y = base * 2;
    int z = base + 10;
    int w = base - 3;
    
    for (int j = 0; j < limit; ++j) {
        /* Different condition pattern */
        if ((j & 0x3) == (base & 0x3)) {
            goto compute_point;
        }
        
        x = y * z;
        y = z + w;
        continue;
        
    compute_point:
        /* Another simple candidate for delay slot */
        z = x & y;  /* Bitwise AND - safe, non-trapping */
        
        /* Follow-up instruction */
        w = z << 2;
    }
    
    return x + y - z + w;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for runtime-dependent values */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Process with different seeds to create variation */
    for (int s = 0; s < 5; ++s) {
        total += process_values(iterations + s, s * 17);
        total += alternate_pattern(iterations / 2 + s, s * 23);
    }
    
    /* Use guard to prevent constant folding */
    if (argc > 2) {
        guard = 1;
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
