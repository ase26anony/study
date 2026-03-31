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
    
    /* Variables for the target instruction - distinct resource set */
    int a = 1, b = 2, c = 3;
    int d = 4, e = 5, f = 6;
    
    /* Variables for post-label operations */
    int x = 7, y = 8, z = 9;
    
    /* Result accumulator to prevent dead code elimination */
    int result = 0;
    
    /*
     * Create a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (i = 0; i < loop_count; ++i) {
        /*
         * Create a runtime-dependent condition that can't be optimized away.
         * Use modulo with a prime number to create irregular branch patterns.
         */
        j = i + cond_base;
        k = j * 3 + 1;
        
        /*
         * KEY CONSTRUCT: Conditional jump to a label.
         * This should generate a simplejump_p instruction.
         * The condition uses variables (j, k) that are NOT used at the target label.
         */
        if ((j + k) % 13 == 0) {
            /* 
             * Use goto to ensure we get a jump to label.
             * The compiler should generate a simple conditional jump.
             */
            goto target_label;
        }
        
        /* Some computation to make the block non-trivial */
        x = y ^ z;
        z = x + 1;
        
        /* Skip the target_label block when not jumping */
        continue;
        
    target_label:
        /*
         * TARGET INSTRUCTION: Simple, safe arithmetic operation.
         * This instruction (next_trial) must:
         * 1. NOT be a jump
         * 2. NOT reference or set resources in &set or &needed
         * 3. NOT be potentially trapping
         * 4. Be eligible for delay slot filling
         * 
         * Using distinct variables (a, b, c) that aren't live across the jump.
         */
        a = b + c;  /* Simple register-to-register operation */
        
        /*
         * Additional operations after the label to ensure the target
         * instruction isn't the only one in its block.
         */
        d = e & f;   /* Another safe operation */
        x = y | z;   /* Using different variables */
        
        /* Continue loop execution */
    }
    
    /*
     * Use all modified variables to create observable side effects
     * and prevent dead code elimination.
     */
    result = a + b + c + d + e + f + x + y + z;
    
    /* Mix with argc to make result runtime-dependent */
    result = result ^ (argc * 37);
    
    printf("Result: %d\n", result);
    
    /* Additional complexity to keep the reorg pass interested */
    {
        int temp = 0;
        for (i = 0; i < (argc % 10) + 5; ++i) {
            temp += i * result;
            
            /* Another conditional jump pattern */
            if (temp % 7 == (argc % 3)) {
                /* Use different variables for another potential target */
                int p = 10, q = 11, r = 12;
                p = q - r;  /* Another safe candidate */
                temp += p;
            }
        }
        result += temp;
    }
    
    printf("Final result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
