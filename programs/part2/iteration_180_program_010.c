/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int test_delay_slot_filling(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    volatile int e = 5;
    volatile int f = 6;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    int result = 0;
    
    /* Create a non-trivial control flow graph with a loop */
    for (int i = 0; i < iterations; ++i) {
        /* Create a conditional jump that's not always true/false */
        if ((i % 7) == (argc % 3)) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Some computation to create register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e - f;
        e = f + a;
        f = a | b;
        
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* This is the candidate for delay slot filling:
           - Simple arithmetic operation
           - No trapping (addition is safe)
           - Uses different registers than the jump condition
           - Not a jump or complex sequence */
        a = b + c;  /* Simple register-to-register operation */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d ^ e;
    }
    
    /* Create observable side-effects to prevent dead code elimination */
    result = a + b + c + d + e + f;
    
    /* Use the result in a way that can't be optimized away */
    if (result > 1000) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 10;
    volatile int s = 20;
    
    /* Nested loop for more complex scheduling context */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Conditional jump based on computation */
            if ((p + q) > (i * j)) {
                goto alt_target;
            }
            
            p = q + r;
            q = r + s;
            r = s - p;
            s = p ^ q;
            
            continue;
            
            alt_target:
            /* Another candidate instruction - bitwise operation is safe */
            p = q & r;  /* Safe, non-trapping operation */
            
            /* Follow-up computation */
            q = r | s;
            r = s << 2;
        }
    }
    
    return p + q + r + s;
}

/* Main function with command-line arguments */
int main(int argc, char **argv) {
    int result1 = test_delay_slot_filling(argc, argv);
    int result2 = alternative_pattern(argc, result1);
    
    /* Final computation that uses both results */
    int final_result = result1 ^ result2;
    
    /* Print based on argc to prevent optimization */
    if (argc > 1) {
        printf("Final: %d\n", final_result);
    }
    
    return final_result % 256;
}
