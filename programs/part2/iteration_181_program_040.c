#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a
 * safe, non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Declare distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int d = 0, e = 0, f = 0;      /* Used at target label */
    int g = 0, h = 0, i = 0;      /* Used in other computations */
    int result = 0;               /* Final checksum */
    
    /* Initialize with runtime-dependent values to prevent constant folding */
    a = (init_val * 3) & 0xFF;
    b = (init_val * 5) & 0xFF;
    c = (init_val * 7) & 0xFF;
    d = (init_val * 11) & 0xFF;
    e = (init_val * 13) & 0xFF;
    f = (init_val * 17) & 0xFF;
    g = (init_val * 19) & 0xFF;
    h = (init_val * 23) & 0xFF;
    i = (init_val * 29) & 0xFF;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_count = (argc > 1) ? 100 : 200;
    for (int iter = 0; iter < loop_count; ++iter) {
        /* Mix different computations to create register pressure */
        g = (g + iter) & 0xFF;
        h = (h ^ iter) & 0xFF;
        i = (i | iter) & 0xFF;
        
        /* 
         * Key construct: Conditional jump with runtime-dependent condition
         * This creates a simplejump_p(trial) that jumps to a label
         */
        if ((iter + argc) % 13 == 0) {
            /* 
             * Use goto instead of if-else to create a simple jump instruction
             * The condition prevents the compiler from optimizing it away
             */
            if ((a + b) > c) {
                goto target_label;
            }
        }
        
        /* Continue with other operations if jump not taken */
        a = (a + 1) & 0xFF;
        b = (b - 1) & 0xFF;
        c = (c ^ iter) & 0xFF;
        
        /* Skip the target instruction when jump not taken */
        continue;
        
    target_label:
        /* 
         * Target instruction for delay slot filling:
         * - Simple arithmetic operation (non-trapping)
         * - Uses different variables than the jump condition
         * - Not a jump, not a SEQUENCE pattern
         * - Does not reference condition codes or special registers
         */
        d = e + f;  /* Safe candidate for delay slot */
        
        /* Additional instruction to ensure target isn't isolated */
        g = h & i;
        
        /* Continue loop execution */
        a = (a + d) & 0xFF;
    }
    
    /* 
     * Create observable side-effects using all variables
     * This prevents dead code elimination
     */
    result = a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i;
    
    /* Mix with loop count to make result input-dependent */
    result = (result * loop_count) & 0xFFFF;
    
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to prevent reordering */
    volatile int sink = result;
    (void)sink;
    
    return result == 0 ? 0 : 1;
}
