/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Function with complex control flow to create scheduling opportunities */
int process_values(int argc, char **argv) {
    /* Use volatile to prevent optimization */
    volatile int base = argc;
    
    /* Multiple local variables for resource independence */
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    int result = 0;
    
    /* Loop to create scheduling context */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent constant folding */
        int condition = (i + base) % 13;
        
        /* 
         * KEY CONSTRUCT: Conditional jump with potential delay slot candidate
         * The target instruction (at target_label) should be a simple,
         * non-trapping instruction that doesn't conflict with jump resources
         */
        if (condition == 0) {
            /* 
             * Use goto instead of structured control flow to ensure
             * a simplejump_p instruction is generated
             */
            goto target_label;
        }
        
        /* 
         * Alternative path with different computations to ensure
         * the target instruction isn't the only one in its block
         */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e | f;
        continue;
        
        /* 
         * TARGET LABEL: Place simple, safe instruction here
         * This should be the candidate for delay slot filling
         * - Simple arithmetic (no trapping)
         * - Uses different registers than condition computation
         * - Not a jump or complex sequence
         */
    target_label:
        /* Simple, safe assignment - prime candidate for delay slot */
        e = f + g;  /* This is next_trial in the uncovered code */
        
        /* Additional operations to ensure block has multiple instructions */
        f = g - h;
        g = h << 2;
        h = a ^ b;
        
        /* Use result to prevent dead code elimination */
        result += (a + b + c + d + e + f + g + h) & 0xFF;
    }
    
    /* 
     * Additional control flow to prevent tail merging
     * and preserve the label structure
     */
    if (base % 3 == 1) {
        /* Another conditional jump to create more scheduling context */
        int temp = a + b;
        if (temp > 100) {
            /* Simple computation at another label */
        another_label:
            c = d + e;
            d = e * f;
        } else {
            goto another_label;
        }
    }
    
    /* Final computation using all variables to create observable side effect */
    int final_result = (a ^ b) + (c & d) - (e | f) + (g ^ h);
    
    /* Print to prevent optimization */
    printf("Result: %d (checksum: %d)\n", final_result, result);
    
    return final_result;
}

/* Main function with command-line arguments for runtime variability */
int main(int argc, char **argv) {
    /* Use argc for runtime-dependent behavior */
    int seed = argc;
    
    /* Initialize with non-zero values to avoid trapping */
    int x = seed + 1;
    int y = seed + 2;
    int z = seed + 3;
    
    /* Multiple function calls to create different contexts */
    int sum = 0;
    for (int i = 0; i < 3; ++i) {
        /* Modify argc to create different paths */
        char *dummy_argv[] = {"program", "test"};
        int dummy_argc = (i % 2) + 1;
        
        sum += process_values(dummy_argc, dummy_argv);
        
        /* Additional computations to affect register allocation */
        x = y * z;
        y = z + i;
        z = x ^ y;
    }
    
    /* Return value based on computation to prevent optimization */
    return (sum > 100) ? 0 : 1;
}
