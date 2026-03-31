#include <stdio.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables used in conditions */
    volatile int cond_base = argc;
    
    /* Variables for the jump condition - separate resource set */
    int i, j, k;
    int cond_var1 = argc * 3;
    int cond_var2 = argc * 7;
    
    /* Variables for the target instruction - distinct resource set */
    int a = 1, b = 2, c = 3;
    int d = 4, e = 5, f = 6;
    
    /* Additional variables to prevent elimination */
    int sum1 = 0, sum2 = 0;
    
    /* Loop provides scheduling context and prevents elimination */
    for (i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Complex condition to prevent constant folding */
        j = i + cond_base;
        k = j * 17;
        
        /* 
         * Create a conditional jump that should be recognized as simplejump_p.
         * The condition uses runtime values to prevent optimization.
         */
        if ((k + argc) % 13 == 0) {
            /* 
             * Jump to label where next_trial instruction resides.
             * This goto creates the simple conditional jump pattern.
             */
            goto target_label;
        }
        
        /* Alternate path to make control flow non-trivial */
        a = b - c;
        continue;
        
    target_label:
        /* 
         * This is the candidate instruction for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing/setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         * 
         * Simple register-to-register operation using distinct variables
         * that aren't live across the jump.
         */
        a = b + c;  /* Simple, safe arithmetic - no trapping */
        
        /* Additional operation to ensure target isn't isolated */
        d = e ^ f;
        
        /* Use results to prevent dead code elimination */
        sum1 += a;
        sum2 += d;
        
        /* Reset variables to break live ranges */
        b = i + 1;
        c = i + 2;
        e = i + 3;
        f = i + 4;
    }
    
    /* 
     * Second loop with different pattern to increase chances 
     * of triggering the logic
     */
    for (i = 0; i < 50; ++i) {
        int x = argc * i;
        int y = argc + i;
        
        if ((x ^ y) & 0xF) {
            goto second_target;
        }
        
        /* Some computation to vary the pattern */
        a = b * 2;
        continue;
        
    second_target:
        /* Another candidate instruction pattern */
        d = e | f;
        
        /* Different operation to avoid pattern recognition */
        b = c & 0xFF;
        
        sum1 += d;
        sum2 += b;
        
        /* Reinitialize to break dependencies */
        e = i * 3;
        f = i * 5;
    }
    
    /* Use results to create observable side effect */
    printf("Checksum: %d\n", sum1 + sum2 + argc);
    
    return 0;
}
