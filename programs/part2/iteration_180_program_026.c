/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int test_delay_slot_filling(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Create a loop to provide scheduling context */
    for (int i = 0; i < loop_limit; ++i) {
        /* Create a conditional check based on runtime value */
        /* The condition should not be always true/false to prevent optimization */
        if ((i % 7) == 0) {
            /* This is the critical simple conditional jump */
            /* The compiler should generate a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Some other code to prevent the jump from being optimized away */
        b = c * d;
        c = b - a;
        continue;
        
        /* The target label with a simple, safe instruction */
        /* This instruction should be eligible for delay slot filling */
        target_label:
        /* Simple arithmetic that doesn't trap and uses independent variables */
        a = b + c;  /* Candidate for delay slot - doesn't set CC, stack pointer, etc. */
        
        /* Continue with other operations */
        d = a * 2;
        b = c + d;
    }
    
    /* Use the variables to create observable side-effects */
    result = a + b + c + d;
    
    /* Additional control flow to keep the scheduler interested */
    if (argc > 2) {
        /* Another conditional jump pattern */
        volatile int x = 10;
        volatile int y = 20;
        
        if (result > 100) {
            goto another_label;
        }
        
        x = y * 2;
        return result + x;
        
        another_label:
        y = x + 5;  /* Another potential delay slot candidate */
        result += y;
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
void another_pattern(int iterations) {
    volatile int p = 0;
    volatile int q = 1;
    volatile int r = 2;
    
    for (int j = 0; j < iterations; j++) {
        /* Mix of conditions to create various jump patterns */
        if ((j & 3) == 0) {
            goto pattern_label;
        }
        
        if ((j % 5) == 0) {
            q = r * p;
            continue;
        }
        
        pattern_label:
        /* Simple, safe instruction - register to register operation */
        r = p + q;  /* Good candidate: doesn't trap, simple operation */
        
        p = q - r;
    }
}

int main(int argc, char **argv) {
    int result = test_delay_slot_filling(argc, argv);
    
    /* Call another pattern with runtime-dependent value */
    another_pattern(argc * 10);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
