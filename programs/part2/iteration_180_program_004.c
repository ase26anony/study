/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int test_delay_slots(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a conditional check based on runtime value */
        /* The condition should not be always true/false to prevent optimization */
        if ((i % 7) == 0) {
            /* Simple conditional jump - this should become a simplejump_p */
            /* The goto target must be a label immediately before a simple instruction */
            goto target_label;
        }
        
        /* Some other code to prevent the compiler from merging blocks */
        d = a + b;
        a = b ^ c;
        
        /* Continue after the label */
        continue;
        
    target_label:
        /* This is the candidate instruction for delay slot filling (next_trial) */
        /* Simple, safe arithmetic operation that doesn't trap */
        /* Must not reference/set CC, stack pointer, or other critical resources */
        c = a + b;  /* Simple register-to-register operation */
        
        /* Additional operations to ensure target instruction isn't alone */
        b = c * d;
        a = i & 0xFF;
    }
    
    /* Create observable side-effects using modified variables */
    result = a + b + c + d;
    
    /* Use the result to prevent dead code elimination */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int another_test(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 0;
    volatile int s = 0;
    
    /* Different loop structure */
    for (int j = 0; j < 50; j++) {
        /* Another conditional jump pattern */
        if ((p + j) & 1) {
            goto another_target;
        }
        
        p = q + j;
        q = p ^ j;
        continue;
        
    another_target:
        /* Another candidate instruction - different operation */
        r = p | q;  /* Bitwise OR is safe and non-trapping */
        
        /* Follow-up operations */
        s = r << 2;
        p = s - j;
    }
    
    return p + q + r + s;
}

/* Main function with command-line arguments */
int main(int argc, char **argv) {
    int result1 = test_delay_slots(argc, argv);
    int result2 = another_test(argc, result1);
    
    /* Print results to create observable output */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent optimization */
    return (result1 + result2) & 0xFF;
}
