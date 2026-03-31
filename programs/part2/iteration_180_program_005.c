/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -S -o test.s test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -S -o test.s test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function designed to create the specific pattern needed for delay slot filling */
int fill_delay_slot_pattern(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 0;
    int f = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int divisor = (argc > 2) ? 7 : 11;  /* Prevent constant folding */
    
    /* Create a loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional jump that's not always true/false */
        if (i % divisor == 0) {
            /* Jump to label - this should become a simplejump_p */
            goto target_label;
        }
        
        /* Some computation to use variables and prevent dead code elimination */
        e = b + c;
        f = d - a;
        a = e ^ f;  /* Simple operation that doesn't trap */
        
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* This instruction should be eligible for delay slot filling:
           - Simple arithmetic (no trapping)
           - Uses different registers than the jump condition
           - Not a jump or complex sequence */
        d = b + c;  /* Simple add, no side effects, doesn't trap */
        
        /* Continue with other operations so target isn't the only instruction */
        a = c * d;
        b = a >> 2;
    }
    
    /* Additional computation to create observable side effects */
    int result = a + b + c + d + e + f;
    
    /* Use result to prevent dead code elimination */
    if (argc > 3) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    int r = 0;
    
    /* Nested loops create more scheduling opportunities */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; ++j) {
            /* Conditional jump based on computation */
            if ((i * j + p) % 13 == 0) {
                goto alt_target;
            }
            
            r = p ^ q;
            q = r + i;
            continue;
            
            alt_target:
            /* Candidate for delay slot - register-to-register operation */
            p = q + 1;  /* Simple increment, no trapping */
            
            /* Follow-up instruction */
            r = p | q;
        }
    }
    
    return p + q + r;
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    int result1 = fill_delay_slot_pattern(argc, argv);
    int result2 = alternative_pattern(argc, result1);
    
    /* Use results to prevent optimization */
    if (result1 != result2) {
        printf("Results differ: %d vs %d\n", result1, result2);
    }
    
    return (result1 + result2) % 256;
}
