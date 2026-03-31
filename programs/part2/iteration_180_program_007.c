/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern for delay slot filling */
static int process_values(int argc, char **argv) {
    volatile int seed = argc; /* Prevent constant propagation */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop provides scheduling context and prevents dead code elimination */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp + i;
        
        /* CRITICAL PATTERN: Conditional jump with potential delay slot candidate */
        /* The condition uses runtime values to prevent optimization */
        if ((i + seed) % 7 == 0) {
            /* Simple conditional jump to label */
            goto target_label;
        }
        
        /* Alternative path */
        a = b - c;
        continue;
        
    target_label:
        /* TARGET INSTRUCTION: Simple, safe operation for delay slot filling */
        /* This should compile to a simple arithmetic instruction */
        a = b + c;  /* Candidate for delay slot - no resource conflicts */
        
        /* Additional operation ensures target isn't isolated */
        b = c * d;
        
        /* Use result to prevent elimination */
        result += a + b;
    }
    
    /* Post-loop operations create observable side effects */
    result = result ^ a ^ b ^ c ^ d;
    
    /* Additional control flow to keep scheduler interested */
    if (seed % 2 == 0) {
        result += process_values(argc - 1, argv);
    }
    
    return result;
}

/* Second function with similar pattern but different register usage */
static int alternate_pattern(int x, int y) {
    int p = x, q = y, r = x + y, s = x - y;
    volatile int limit = x % 10 + 5;
    
    for (int j = 0; j < limit; ++j) {
        /* Create register dependencies */
        int tmp = p * q;
        s = tmp + j;
        
        /* Another conditional jump pattern */
        if ((j + x) % 11 == 0) {
            goto alt_target;
        }
        
        r = p - q;
        continue;
        
    alt_target:
        /* Different but safe operation */
        p = q + r;  /* Another delay slot candidate */
        
        q = r * s;
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Initial processing */
    total += process_values(argc, argv);
    
    /* Alternate path */
    total += alternate_pattern(argc, total);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
