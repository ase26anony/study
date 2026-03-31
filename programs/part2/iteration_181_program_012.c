#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Declare two distinct sets of variables to ensure resource independence */
    /* Set 1: Used for the jump condition */
    int cond_a, cond_b, cond_result;
    
    /* Set 2: Used at the target label (must not conflict with &set/&needed resources) */
    int target_x, target_y, target_z;
    int compute_m, compute_n, compute_o;
    
    /* Additional variables to create register pressure and prevent elimination */
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    cond_a = init_val * 3 + 7;
    cond_b = init_val * 5 + 11;
    
    target_x = init_val * 2 + 1;
    target_y = init_val * 7 + 3;
    target_z = 0;
    
    compute_m = init_val * 11 + 5;
    compute_n = init_val * 13 + 7;
    compute_o = 0;
    
    /* Create a loop to provide scheduling context and prevent dead code elimination */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_count; ++i) {
        /* 
         * Create a runtime-dependent condition to prevent if-conversion.
         * Use modulo with a prime number to create unpredictable but safe branching.
         */
        cond_result = (i + argc) % 13;
        
        /* 
         * KEY CONSTRUCT: Conditional jump to label.
         * This should generate a simplejump_p instruction.
         * The condition uses only local variables that are not live across the jump.
         */
        if (cond_result == 0) {
            /* 
             * Use goto instead of function call to create a direct jump to label.
             * This prevents tail call optimization and preserves the jump structure.
             */
            goto delay_slot_candidate;
        }
        
        /* 
         * Code after the if-block to ensure the jump isn't the last instruction.
         * Use different variables than those at the target label.
         */
        temp1 = compute_m + compute_n;
        temp2 = temp1 ^ i;
        
        /* Skip the target label code when not jumping */
        continue;
        
delay_slot_candidate:
        /*
         * TARGET INSTRUCTION: This should be the 'next_trial' instruction.
         * Requirements:
         * 1. Not a jump
         * 2. Single RTL pattern (not SEQUENCE)
         * 3. Safe operation (no trapping)
         * 4. Uses variables not live across the jump
         * 5. Doesn't reference/set condition codes or special registers
         */
        target_z = target_x + target_y;  /* Simple register-to-register add */
        
        /* 
         * Additional instruction after the target to ensure it's not alone.
         * Use bitwise operation which is also safe.
         */
        compute_o = target_x & target_y;
        
        /* Continue loop execution */
        temp3 = target_z * 2;
        temp4 = compute_o + i;
    }
    
    /* 
     * Use all variables in final computation to prevent dead code elimination.
     * Create observable side effect.
     */
    int checksum = cond_a + cond_b + target_z + compute_o + temp1 + temp2;
    
    /* Mix in loop counter to make result dependent on execution path */
    checksum ^= loop_count;
    
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    /* Additional volatile operations to prevent reordering */
    volatile int sink = checksum;
    (void)sink;
    
    return checksum & 0xFF;
}
