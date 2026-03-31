/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_seed = 42;

/* Function to create runtime-dependent values */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int limit = (argc > 1) ? 100 : 200;
    
    /* Mix in global volatile to prevent constant folding */
    int mod_base = global_seed % 10;
    if (mod_base == 0) mod_base = 7;
    
    /* Main loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional that's not always true/false */
        if ((i % mod_base) == 0) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Some computations to create register pressure */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* This should be the candidate for delay slot filling */
        /* Simple arithmetic, no trapping, no resource conflicts */
        a = b + c;  /* next_trial candidate */
        
        /* Continue with more operations so it's not alone */
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        /* Add another conditional to prevent straight-line optimization */
        if ((i & 1) == 0) {
            a = b - c;
        }
    }
    
    /* Use the computed values to create observable side effects */
    result = a + b + c + d;
    
    /* Additional loop to increase complexity */
    for (int j = 0; j < 50; ++j) {
        /* More conditionals with gotos */
        if ((result % 11) == (j % 5)) {
            int temp = a;
            a = b;
            b = temp;
            goto another_label;
        }
        
        result = result * 3 + j;
        continue;
        
        another_label:
        /* Another candidate for delay slot filling */
        c = d + a;  /* Simple, safe instruction */
        d = b - c;
    }
    
    return result;
}

/* Second function to create more opportunities */
int secondary_processing(int x, int y) {
    int p = x, q = y, r = 0, s = 0;
    
    /* Nested loops with conditionals */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 15; ++j) {
            /* Conditional jump to label */
            if ((p + q + i + j) % 13 == 0) {
                goto process_label;
            }
            
            p = q + i;
            q = p - j;
            continue;
            
            process_label:
            /* Candidate instruction - register move/arithmetic */
            r = s + p;  /* Should be eligible for delay slot */
            s = q - r;
            
            /* Prevent optimization */
            if (r > s) {
                p = r * 2;
            }
        }
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int result1, result2, final_result;
    
    /* Process with command-line dependent values */
    result1 = process_values(argc, argv);
    
    /* Use argc for different code paths */
    if (argc > 2) {
        result2 = secondary_processing(argc, atoi(argv[1]));
    } else {
        result2 = secondary_processing(global_seed, argc * 10);
    }
    
    final_result = result1 ^ result2;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Use result in return to ensure it's not optimized away */
    return (final_result % 256);
}
