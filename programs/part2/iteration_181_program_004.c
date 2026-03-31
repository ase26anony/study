#include <stdio.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is:
 * 1. A simple arithmetic operation (not a jump)
 * 2. Does not reference or modify critical resources
 * 3. Is not potentially trapping
 * 4. Is eligible for delay slot filling
 */

int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent values to prevent optimization */
    int base = argc > 0 ? argc : 1;
    
    /* Declare distinct sets of variables to ensure resource independence */
    /* Variables for the jump condition */
    int cond_var1 = base + 1;
    int cond_var2 = base + 2;
    
    /* Variables for the target instruction (must be independent) */
    int target_var1 = base + 3;
    int target_var2 = base + 4;
    int target_result = 0;
    
    /* Additional variables to prevent elimination */
    int extra_var1 = base + 5;
    int extra_var2 = base + 6;
    int extra_result = 0;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_limit = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < loop_limit; ++i) {
        /* Create a runtime-dependent condition that can't be optimized away */
        /* Using modulo with a non-power-of-two to prevent optimization */
        int condition = (i + base) % 13;
        
        /* 
         * The key construct: conditional jump to a label
         * This should create a simplejump_p instruction
         */
        if (condition == 0) {
            /* 
             * Use goto instead of if-else to ensure we get a jump to label
             * rather than conditional move or other transformations
             */
            goto target_label;
        }
        
        /* Some other computation to make the block non-trivial */
        extra_result = extra_var1 ^ extra_var2;
        checksum += extra_result;
        
        /* Skip the target instruction when we don't jump */
        continue;
        
    target_label:
        /*
         * Target instruction for delay slot filling:
         * Simple, safe arithmetic operation that:
         * 1. Is not a jump
         * 2. Uses independent variables (not used in jump condition)
         * 3. Is not potentially trapping (no division, no null pointer)
         * 4. Should be a single RTL pattern (not SEQUENCE)
         */
        target_result = target_var1 + target_var2;
        
        /* Additional operation to ensure target block has multiple instructions */
        extra_result = target_var1 & target_var2;
        checksum += target_result + extra_result;
        
        /* Modify target variables to prevent CSE and ensure they're live */
        target_var1 += 1;
        target_var2 += 2;
    }
    
    /* 
     * Additional loop with different pattern to increase chances 
     * of the reorg pass seeing our construct
     */
    for (int j = 0; j < loop_limit / 2; ++j) {
        /* Another conditional jump pattern */
        if ((j * base) % 7 == 0) {
            /* Force a jump to different label */
            goto second_target;
        }
        
        extra_result = extra_var1 | extra_var2;
        checksum += extra_result;
        continue;
        
    second_target:
        /* Another safe target instruction */
        target_result = target_var1 - target_var2;
        extra_result = target_var1 ^ target_var2;
        checksum += target_result * 2 + extra_result;
        
        /* Modify variables to prevent optimization */
        target_var1 ^= 1;
        target_var2 ^= 2;
    }
    
    /* Use all variables to create observable side effect */
    int final_result = checksum + target_result + extra_result + cond_var1 + cond_var2;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
