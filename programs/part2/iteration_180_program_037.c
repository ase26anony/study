/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and complex control flow */
int test_delay_slot_filling(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    volatile int e = 5;
    volatile int f = 6;
    
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_count = (argc > 1) ? 100 : 200;
    
    /* Complex loop to provide scheduling context */
    for (int i = 0; i < loop_count; ++i) {
        /* Create a non-trivial condition that can't be optimized away */
        int condition = (i * 17 + argc) % 13;
        
        /* The key construct: conditional jump to label */
        if (condition == 0) {
            /* This goto should create a simple conditional jump */
            goto target_label;
        }
        
        /* Some computations to create register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e + f;
        e = f - a;
        f = a | b;
        
        /* Skip the target code when not jumping */
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* This is the candidate for delay slot filling */
        /* Simple arithmetic that doesn't trap and uses different registers */
        a = b + c;  /* Should compile to simple add instruction */
        
        /* Continue with other operations so target isn't isolated */
        b = c * d;
        c = d + 1;
    }
    
    /* Use the variables to create observable side effects */
    result = a + b + c + d + e + f;
    
    /* Additional control flow to prevent dead code elimination */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 10;
    volatile int s = 20;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            /* Multiple conditions to create different jump paths */
            if ((i + j) % 11 == 0) {
                if (p > q) {
                    goto alt_target;
                }
            }
            
            p = q + r;
            q = r - s;
            r = s * p;
            s = p ^ q;
            
            continue;
            
            alt_target:
            /* Another candidate instruction for delay slot */
            p = q + r;  /* Simple add, different registers from condition */
            
            /* Follow-up instructions */
            q = r + 1;
            r = s - 2;
        }
    }
    
    return p + q + r + s;
}

/* Main function to drive the test */
int main(int argc, char **argv) {
    int result1 = test_delay_slot_filling(argc, argv);
    int result2 = alternative_pattern(argc, result1);
    
    /* Print results to prevent complete optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Return non-zero to make execution observable */
    return (result1 + result2) != 0 ? 0 : 1;
}
