#include <stdio.h>

/* 
 * This program is crafted to trigger GCC's delay slot filling logic
 * in the reorg pass. It creates a specific pattern where:
 * 1. A conditional jump (simplejump_p) targets a label
 * 2. The instruction at the label is a safe, non-jump candidate
 * 3. The candidate doesn't reference critical resources
 * 4. The candidate isn't potentially trapping
 */

int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent values */
    int base = argc;
    
    /* Variables for the jump condition - kept separate */
    int cond_var1 = base * 3;
    int cond_var2 = base + 7;
    
    /* Variables for the target instruction - distinct set */
    int target_var1 = base * 5;
    int target_var2 = base * 11;
    int target_var3 = 0;
    
    /* Additional variables to prevent optimization */
    int loop_var1 = base * 13;
    int loop_var2 = base * 17;
    int result = 0;
    
    /* Volatile variables to prevent constant folding */
    volatile int vol1 = argc;
    volatile int vol2 = argc * 2;
    
    /* 
     * Loop provides scheduling context and prevents 
     * elimination of the jump pattern
     */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* 
         * Create a runtime-dependent condition that can't be 
         * optimized away. Using volatile ensures the compiler
         * can't predict the outcome.
         */
        int condition = (i + vol1) % 13;
        
        /* 
         * The key construct: a conditional jump to a label.
         * The condition uses variables that are NOT used
         * in the target instruction.
         */
        if ((condition + cond_var1 - cond_var2) == 0) {
            /* 
             * This goto creates a simplejump_p instruction
             * that jumps to target_label
             */
            goto target_label;
        }
        
        /* 
         * Some computation after the if to ensure the
         * target_label isn't the only instruction in its block
         */
        loop_var1 = loop_var1 ^ (i * 3);
        loop_var2 = loop_var2 + (i & 0xF);
        
        /* Skip the target_label code when not jumping */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A simple, non-jump instruction
         * 2. Not reference resources in &set or &needed
         * 3. Not potentially trapping
         * 4. Eligible for delay slot filling
         * 
         * Using simple arithmetic on distinct variables
         */
        target_var3 = target_var1 + target_var2;
        
        /* 
         * Additional instruction after the label to ensure
         * it's not a single-instruction block
         */
        loop_var1 = loop_var1 ^ target_var3;
    }
    
    /* 
     * Use all variables to create observable side effects
     * and prevent dead code elimination
     */
    result = cond_var1 + cond_var2 + target_var1 + 
             target_var2 + target_var3 + loop_var1 + loop_var2;
    
    /* Mix in some more operations to affect scheduling */
    for (int j = 0; j < 10; ++j) {
        result = (result << 3) | (result >> 5);
        result ^= vol2;
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
