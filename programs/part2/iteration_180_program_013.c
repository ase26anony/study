/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern */
int process_values(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int base = argc;
    int a = 0, b = 1, c = 2, d = 3;
    int result = 0;
    
    /* Create runtime-dependent loop bound */
    int limit = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        a = temp + d;
        
        /* Critical pattern: conditional jump to label */
        /* Use modulo with prime to prevent optimization */
        if ((i % 7) == (base & 3)) {
            /* This goto creates a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Alternative path */
        d = a - b;
        continue;
        
        /* Target label with simple, safe instruction */
        /* This should become next_trial in reorg.cc */
        target_label:
        /* Simple arithmetic - no trapping, no special registers */
        a = b + c;  /* Candidate for delay slot filling */
        
        /* Continue with other operations */
        b = c * d;
        c = d + i;
    }
    
    /* Use results to prevent dead code elimination */
    result = a + b + c + d;
    
    /* Additional control flow to keep scheduler interested */
    if (argc > 2) {
        result *= 2;
    } else {
        result /= 2;  /* Safe division by constant 2 */
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chance */
int alternate_pattern(int x, int y) {
    int p = x, q = y, r = 0, s = 0;
    
    /* Nested loops create more scheduling opportunities */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; ++j) {
            /* Another conditional jump pattern */
            if ((i + j) & 1) {
                goto alt_target;
            }
            
            p = q + i;
            continue;
            
            alt_target:
            /* Another candidate instruction - register move pattern */
            r = p;  /* Simple move that should be safe */
            
            q = r + j;
            s = p ^ q;
        }
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int result1 = process_values(argc, argv);
    int result2 = alternate_pattern(argc, result1);
    
    /* Print to create observable side effect */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Return value based on both computations */
    return (result1 + result2) & 0xFF;
}
