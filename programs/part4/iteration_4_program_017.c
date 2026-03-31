/* Target: MIPS with delay slot filling */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 1;
    volatile int mem2 = 2;
    volatile int mem3 = 3;
    volatile int mem4 = 4;
    
    /* Explicit register variables to control resource allocation */
    register int r0 asm("t0") = 0;  /* Candidate delay slot register */
    register int r1 asm("t1") = 1;  /* Independent register for next_trial */
    register int r2 asm("t2") = 2;  /* Loop counter */
    register int r3 asm("t3") = 3;  /* Accumulator */
    register int r4 asm("t4") = 4;  /* Another independent register */
    register int r5 asm("t5") = 5;  /* Yet another independent register */
    
    int result = 0;
    int i;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic in delay slot position */
        r0 = mem1 + mem2;  /* This could be moved into delay slot */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r2 > 50) {
            /* Use goto to ensure label jump */
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = r3 + mem3;
        
        /* Continue to next pattern */
        goto next_pattern;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (r1, r4) than delay slot candidate (r0) */
        r1 = r4 * 2;  /* Simple arithmetic, non-trapping */
        result += r1;
        
    next_pattern:
        /* Pattern 2: Another conditional jump with different resources */
        r5 = mem4 - mem1;  /* Another delay slot candidate */
        
        if (r2 < 25) {
            goto L2;
        }
        
        r3 = r3 - mem2;
        goto pattern3;
        
    L2:
        /* Another independent instruction after label */
        /* Uses r4 which doesn't conflict with r5 */
        r4 = r1 & 0xFF;  /* Bitwise operation, cannot trap */
        result += r4;
        
    pattern3:
        /* Pattern 3: Nested conditionals to create multiple jump opportunities */
        r0 = r3 << 2;  /* Shift operation as delay slot candidate */
        
        if (i % 3 == 0) {
            goto L3;
        }
        
        if (i % 5 == 0) {
            goto L4;
        }
        
        r2 = r2 + 1;
        continue;
        
    L3:
        /* Independent load operation after label */
        /* Volatile ensures it's not optimized away */
        r1 = mem1;
        result += r1;
        continue;
        
    L4:
        /* Simple arithmetic with different register */
        r4 = r5 + 7;
        result += r4;
        
        /* Modify loop counter to create varying conditions */
        r2 = (r2 + i) & 0x7F;
    }
    
    /* Pattern 4: Function call to create different control flow */
    {
        register int a asm("t6") = 10;
        register int b asm("t7") = 20;
        
        a = a * b;  /* Delay slot candidate */
        
        if (result > 1000) {
            goto L5;
        }
        
        b = b / 2;
        goto final;
        
    L5:
        /* Independent store operation */
        mem1 = a + b;  /* Volatile store, cannot be moved */
        result += mem1;
    }
    
final:
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r0, r1, r2, r3, r4, r5);
    
    return result != 0;
}
