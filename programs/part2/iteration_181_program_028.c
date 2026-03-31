#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the reorg pass for architectures with delay slots (like MIPS).
 * It creates a conditional jump to a label where the target instruction
 * is a safe candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int a = 0, b = 0, c = 0;
    volatile int d = 0, e = 0, f = 0;
    volatile int x = 0, y = 0, z = 0;
    volatile int result = 0;
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = argc + 1;
    b = argc * 2;
    c = argc + 3;
    d = argc * 4;
    e = argc + 5;
    f = argc * 6;
    x = argc + 7;
    y = argc * 8;
    z = argc + 9;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition to prevent optimization.
         * The modulo operation with a non-power-of-two prevents simplification.
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * Simple conditional jump to label.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* Some other computations to create register pressure */
        x = y ^ z;
        y = x + i;
        z = y & 0xFF;
        
        continue;
        
        /* 
         * TARGET LABEL: The instruction immediately after this label
         * should be a candidate for delay slot filling.
         * Requirements:
         * 1. Not a jump
         * 2. Single RTL pattern (not SEQUENCE)
         * 3. Doesn't reference/set critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
    target_label:
        /* Safe, non-trapping arithmetic operation using DIFFERENT variables */
        a = b + c;  /* This is the candidate instruction for delay slot */
        
        /* Additional operations to ensure the block isn't trivial */
        d = e ^ f;
        e = d + 1;
        f = e & 0x7F;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * and prevent dead code elimination
     */
    result = a + b + c + d + e + f + x + y + z;
    
    /* Print result to create observable behavior */
    printf("Result: %d\n", result);
    
    /* Additional complexity to survive optimization passes */
    if (argc > 2) {
        volatile int temp = result;
        for (int j = 0; j < 10; ++j) {
            temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        printf("Processed: %d\n", temp);
    }
    
    return result & 1;
}
