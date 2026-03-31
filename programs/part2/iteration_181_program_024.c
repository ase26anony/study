#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables used in conditions */
    volatile int cond_base = argc;
    
    /* Variables for the jump condition - separate resource set */
    int i, j, k;
    int cond_var1 = argc * 3;
    int cond_var2 = argc + 7;
    
    /* Variables for the target instruction - completely separate resource set */
    int a = 1, b = 2, c = 3;
    int d = 4, e = 5, f = 6;
    
    /* Additional variables to prevent elimination */
    int sum1 = 0, sum2 = 0;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent compile-time optimization */
        j = i + cond_base;
        k = (j * 7) % 13;
        
        /* 
         * Key construct: Conditional jump that should be recognized as simplejump_p.
         * The condition uses variables that are separate from the target instruction.
         */
        if ((k + cond_var1 - cond_var2) % 11 == 0) {
            /* 
             * Use goto instead of if-else to create a simple jump to label.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* Some computation to make the block non-trivial */
        sum1 += i * 2;
        continue;
        
    target_label:
        /*
         * Target instruction for delay slot filling.
         * This is a simple, safe arithmetic operation that:
         * 1. Is not a jump (non-jump instruction)
         * 2. Uses variables separate from the jump condition
         * 3. Should not reference or set critical resources (&set, &needed)
         * 4. Is not potentially trapping (safe integer addition)
         * 5. Should be eligible for delay slot filling
         */
        a = b + c;  /* Simple register-to-register operation */
        
        /* Additional computation to ensure target isn't isolated */
        d = e ^ f;  /* Bitwise operation, also safe */
        
        /* Continue loop computation */
        sum2 += a + d;
    }
    
    /* 
     * Use the results to create observable side effects.
     * This prevents dead code elimination.
     */
    int result = sum1 + sum2 + a + b + c + d + e + f;
    
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to prevent reordering */
    volatile int check = result % 256;
    if (check == 42) {
        printf("Unexpected condition met\n");
    }
    
    return result > 0 ? 0 : 1;
}
