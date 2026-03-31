#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe
 * candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent constant folding and dead code elimination */
    volatile int init = argc;
    
    /* Declare multiple sets of variables to ensure resource independence */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int x = 0, y = 0, z = 0, w = 0, v = 0, u = 0;
    
    /* Initialize with runtime-dependent values */
    a = init + 1;
    b = init + 2;
    c = init + 3;
    d = init + 4;
    e = init + 5;
    f = init + 6;
    x = init + 7;
    y = init + 8;
    z = init + 9;
    w = init + 10;
    v = init + 11;
    u = init + 12;
    
    /* Result accumulator to create observable side effects */
    int result = 0;
    
    /*
     * Create a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * First set of operations using one group of variables.
         * These create register pressure and scheduling opportunities.
         */
        a = b + c;      /* Simple arithmetic - safe */
        d = e ^ f;      /* Bitwise operation - safe */
        x = y | z;      /* Another bitwise operation */
        
        /*
         * KEY CONSTRUCT: Conditional jump with safe target instruction
         * 
         * The condition uses runtime values to prevent optimization.
         * The jump target is a label placed immediately before a simple
         * arithmetic operation using a DIFFERENT set of variables.
         */
        if ((i + init) % 13 == 0) {
            /* 
             * This should generate a simple conditional jump (simplejump_p).
             * The target is 'target_label' where we have a safe instruction.
             */
            goto target_label;
        }
        
        /* Continue with other operations if jump not taken */
        w = v - u;      /* Safe subtraction */
        result += a + d + x + w;
        
        /* Skip the target section when not jumping */
        goto continue_loop;
        
    target_label:
        /*
         * TARGET INSTRUCTION CANDIDATE for delay slot filling.
         * This must be:
         * 1. A non-jump instruction
         * 2. Not referencing or setting critical resources (&set, &needed)
         * 3. Not potentially trapping
         * 4. A single RTL pattern (not SEQUENCE)
         * 5. Eligible for delay slot
         * 
         * Using different variables than the jump condition ensures
         * resource independence.
         */
        a = b + c;      /* Simple register-to-register operation */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        d = e ^ f;
        x = y | z;
        result += a + d + x;
        
    continue_loop:
        /* More operations to create complex control flow */
        if ((i + init) % 7 == 0) {
            v = u << 2;     /* Safe shift operation */
        } else {
            v = u >> 1;     /* Safe shift operation */
        }
        
        /* Mix in some additional computations */
        result = (result * 31) & 0xFFFF;
    }
    
    /* 
     * Additional computations using all variables to ensure they're live
     * and prevent optimization of the entire construct.
     */
    int final_result = result + a + b + c + d + e + f + x + y + z + w + v + u;
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
