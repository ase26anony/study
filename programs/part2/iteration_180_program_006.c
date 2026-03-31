/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -S -o test.s test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -S -o test.s test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function designed to create specific instruction patterns for delay slot filling */
int target_function(int argc, char **argv) {
    /* Use volatile to prevent optimization */
    volatile int seed = argc;
    
    /* Declare multiple integer variables to create register pressure */
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    int result = 0;
    
    /* Create a loop with runtime-dependent iteration count */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix operations to create scheduling opportunities */
        a = b + c;
        b = c ^ d;
        c = d | e;
        d = e & f;
        
        /* Create a conditional jump that's not trivially predictable */
        /* Use modulo with prime to prevent optimization */
        if ((i % 7) == (seed % 3)) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Alternative path with different operations */
        e = f * g;
        f = g - h;
        continue;
        
        /* Target label with simple, safe instruction */
        /* This should be the candidate for delay slot filling */
        target_label:
        /* Simple, non-trapping instruction: register-to-register move */
        /* Must not reference CC, stack pointer, or conflict with jump resources */
        h = a;  /* Simple move that's safe for delay slot */
        
        /* Continue with more operations so target isn't isolated */
        g = h + 1;
    }
    
    /* Create observable side effects to prevent dead code elimination */
    result = a + b + c + d + e + f + g + h;
    
    /* Use result in a way that can't be optimized away */
    if (argc > 2) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternate_pattern(int x, int y) {
    volatile int trigger = x;
    int p = x, q = y, r = 0, s = 0;
    
    /* Nested loops create more scheduling context */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Create another conditional jump opportunity */
            if ((i * j) % 11 == trigger % 5) {
                goto alt_target;
            }
            
            p = q + i;
            q = p - j;
            continue;
            
            alt_target:
            /* Another simple, safe instruction candidate */
            r = p;  /* Register move */
            
            s = r * 2;
        }
    }
    
    return p + q + r + s;
}

/* Main function to drive execution */
int main(int argc, char **argv) {
    int result1 = target_function(argc, argv);
    int result2 = alternate_pattern(argc, result1);
    
    /* Final result depends on both functions */
    int final_result = result1 ^ result2;
    
    /* Print to prevent optimization */
    if (argc > 1) {
        printf("Final: %d\n", final_result);
    }
    
    return final_result % 256;
}
