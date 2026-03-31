/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and complex control flow */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 0;
    int f = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Create a complex loop to provide scheduling context */
    for (int i = 0; i < loop_limit; ++i) {
        /* Mix different operations to create register pressure */
        e = a + b;
        f = c - d;
        
        /* Create a conditional jump that might be optimized for delay slot filling */
        /* The condition uses runtime values to prevent constant folding */
        if ((i % 7) == (argc % 3)) {
            /* This goto creates a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Alternative path with different operations */
        a = b * c;
        b = c + d;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling */
        /* Simple arithmetic that doesn't trap and uses different registers than jump condition */
        d = e + f;  /* This should be eligible for delay slot */
        
        /* Continue with other operations to ensure target isn't isolated */
        c = a - b;
        a = d * 2;
    }
    
    /* Additional code to prevent dead code elimination */
    if (argc > 2) {
        /* Create another conditional jump pattern */
        int x = atoi(argv[1]);
        int y = atoi(argv[2]);
        
        if (x != y) {
            /* Another potential delay slot candidate */
            volatile int temp = x + y;
            x = temp - 1;
        }
        
        /* Return computed value to create observable side effect */
        return a + b + c + d + e + f + x + y;
    }
    
    /* Final computation using all variables to prevent optimization */
    int result = a ^ b ^ c ^ d ^ e ^ f;
    
    /* Print to create side effect */
    printf("Result: %d\n", result);
    
    return result;
}

/* Main function with command line arguments */
int main(int argc, char **argv) {
    /* Call the test function multiple times with different arguments */
    int sum = 0;
    
    for (int j = 0; j < 3; ++j) {
        /* Modify argc locally to vary behavior */
        int modified_argc = argc + j;
        char *modified_argv[] = {argv[0], "10", "20", NULL};
        
        sum += fill_delay_slot_test(
            (modified_argc > 3) ? 3 : modified_argc,
            modified_argv
        );
    }
    
    /* Final output to ensure all code is live */
    printf("Final sum: %d\n", sum);
    
    return (sum != 0) ? 0 : 1;
}
