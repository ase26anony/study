/* Target: MIPS with delay slots - designed to trigger specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile memory locations to create specific resource patterns */
volatile int mem1 = 100;
volatile int mem2 = 200;
volatile int mem3 = 300;
volatile int mem4 = 400;

int main() NOMIPS16 {
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");  /* $t0 - temp register 0 */
    register int r2 asm("t1");  /* $t1 - temp register 1 */
    register int r3 asm("t2");  /* $t2 - temp register 2 */
    register int r4 asm("t3");  /* $t3 - temp register 3 */
    register int r5 asm("t4");  /* $t4 - temp register 4 */
    register int r6 asm("t5");  /* $t5 - temp register 5 */
    
    int result = 0;
    int i;
    
    /* Initialize registers with distinct values */
    r1 = 1;
    r2 = 2;
    r3 = 3;
    r4 = 4;
    r5 = 5;
    r6 = 6;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 1000; i++) {
        /* BLOCK 1: Simple conditional jump with independent instruction after label */
        /* This creates a candidate for delay slot filling */
        if (r1 > 0) {
            /* Potential delay slot candidate: uses r1, sets r2 */
            r2 = r1 + mem1;  /* Independent of what's after label1 */
            goto label1;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = r4 * 2;
        
    label1:
        /* This is next_trial: independent instruction after jump label */
        /* Uses r5, sets r6 - doesn't conflict with r1/r2 above */
        r6 = r5 & 0xFF;  /* Simple arithmetic, cannot trap */
        result += r6;
        
        /* BLOCK 2: Another conditional jump with different registers */
        if (r3 < 100) {
            /* Another delay slot candidate */
            r4 = r3 - mem2;  /* Uses r3, sets r4 */
            goto label2;
        }
        
        r1 = r2 + 10;
        
    label2:
        /* Another independent instruction after label */
        r5 = r6 | 0x55;  /* Simple bitwise operation */
        result += r5;
        
        /* BLOCK 3: More complex to increase slots_to_fill analysis */
        if (mem3 != 0) {
            /* Volatile load as delay slot candidate */
            r1 = mem3;  /* Load from volatile memory */
            goto label3;
        }
        
        r2 = r4 - 5;
        
    label3:
        /* Independent arithmetic after label */
        r3 = r1 * 2;  /* Multiplication is still simple arithmetic */
        result += r3;
        
        /* BLOCK 4: Nested condition to create more jump opportunities */
        if (i % 2 == 0) {
            if (r2 > r1) {
                r4 = r2 << 1;  /* Shift operation */
                goto label4;
            }
            r5 = r3 + 1;
        }
        
        r6 = mem4;
        
    label4:
        /* Final independent operation */
        r1 = r4 ^ r5;  /* XOR operation */
        result += r1;
        
        /* Modify registers to change conditions */
        r1 += i;
        r2 -= i % 3;
        r3 = (r3 * 3) % 100;
        mem1++;  /* Change volatile memory to affect conditions */
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r1, r2, r3, r4, r5, r6);
    
    return 0;
}
