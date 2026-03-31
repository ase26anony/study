/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern for delay slot filling */
int process_values(int argc, char **argv) {
    volatile int seed = argc; /* Prevent constant propagation */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp - a;
        
        /* Critical pattern: conditional jump with potential delay slot candidate */
        if ((i + seed) % 7 == 0) {
            /* Simple conditional jump that should become simplejump_p */
            if (a != 0) {
                /* Jump to label where candidate instruction resides */
                goto target_label;
            }
        }
        
        /* Alternative path computations */
        a = b + 1;
        b = c * 2;
        continue;
        
        /* Target label with simple, safe instruction as delay slot candidate */
        target_label:
        /* Candidate instruction: simple arithmetic, no trapping, no resource conflicts */
        /* Uses different registers than the jump condition (a != 0) */
        c = d + b;  /* This should be eligible for delay slot filling */
        
        /* Continue with other operations */
        a = temp % 5;
        if (a == 0) a = 1; /* Avoid division by zero later */
    }
    
    /* Use all variables to prevent dead code elimination */
    result = a + b * 2 - c / 3 + d;
    
    /* Additional control flow to keep scheduler interested */
    for (int j = 0; j < 10; ++j) {
        if (result % (j + 2) == 0) {
            b += j;
        } else {
            c -= j;
        }
    }
    
    return result + a - b + c * d;
}

/* Second function with different pattern to increase coverage chances */
int alternate_pattern(int x, int y) {
    int p = x, q = y, r = 0, s = 0;
    
    /* Nested loops for complex control flow */
    for (int i = 0; i < 50; ++i) {
        r = p * q;
        
        /* Another conditional jump pattern */
        if (r % 11 == (i % 3)) {
            if (p > q) {
                goto alt_target;
            }
        }
        
        s = r + i;
        p = q - i;
        q = p + 1;
        continue;
        
        alt_target:
        /* Another candidate instruction - register move pattern */
        s = p + q;  /* Simple addition, safe for delay slot */
        
        r = s * 2;
        q = r - p;
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int result1 = process_values(argc, argv);
    int result2 = alternate_pattern(argc, result1);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Return non-deterministic value based on inputs */
    return (result1 + result2) % 256;
}
