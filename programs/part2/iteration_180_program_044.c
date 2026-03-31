/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile for some to prevent optimization */
    volatile int init = argc;
    int a = init * 2;
    int b = init + 5;
    int c = init - 3;
    int d = init * init;
    int e = 0;
    int f = 0;
    
    /* Use argc to determine loop bounds - prevents constant folding */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Create a non-trivial control flow with a loop */
    for (int i = 0; i < loop_limit; ++i) {
        /* Mix different operations to create register pressure */
        e = a + b;
        f = c ^ d;
        
        /* Key construct: conditional jump with potential delay slot candidate */
        /* The condition is runtime-dependent to prevent dead code elimination */
        if ((i % 7) == (init & 3)) {
            /* This goto creates a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Alternative path with different computations */
        a = b + 1;
        b = c * 2;
        c = d - a;
        d = e ^ f;
        
        /* Skip the target code if we didn't jump */
        continue;
        
        /* Target label with simple, safe instruction for delay slot filling */
        target_label:
        /* Candidate instruction for delay slot: simple arithmetic, no traps */
        /* Uses different registers than the jump condition (i, init) */
        a = b + c;  /* This should be eligible for delay slot */
        
        /* Continue with other operations so target isn't isolated */
        b = c * d;
        c = a ^ b;
    }
    
    /* Additional code to use all variables, preventing dead store elimination */
    int result = a + b + c + d + e + f;
    
    /* Create observable side effect */
    printf("Result: %d\n", result);
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int another_test_case(int x, int y) {
    int p = x * 2;
    int q = y + 10;
    int r = x - y;
    int s = 0;
    
    /* Loop with multiple potential jump targets */
    for (int j = 0; j < 50; ++j) {
        s = p + q;
        
        /* Different condition pattern */
        if ((j & 1) == (x & 1)) {
            goto another_target;
        }
        
        p = q + r;
        q = r * 2;
        r = s - p;
        continue;
        
        another_target:
        /* Another candidate instruction - register move pattern */
        r = p + q;  /* Simple add, should be safe */
        
        /* Follow with more operations */
        p = q ^ r;
    }
    
    return p + q + r + s;
}

/* Main function with command line arguments */
int main(int argc, char **argv) {
    int result1 = fill_delay_slots_test(argc, argv);
    
    /* Use argv to create different test values */
    int x = (argv[0] != 0) ? argc : 10;
    int y = (argc > 0) ? atoi(argv[0]) : 20;
    
    int result2 = another_test_case(x, y);
    
    /* Final result depends on both computations */
    return (result1 + result2) > 0 ? 0 : 1;
}
