/* Target: MIPS with delay slots - triggers specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create memory ops */
    volatile int mem1 = 100;
    volatile int mem2 = 200;
    volatile int mem3 = 300;
    volatile int result = 0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 1;  /* Candidate delay slot reg */
    register int r2 asm("t1") = 2;  /* Independent reg for next_trial */
    register int r3 asm("t2") = 3;  /* Another independent reg */
    register int r4 asm("t3") = 4;
    register int cond asm("t4") = 0;
    
    int i;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple arithmetic that could fill delay slot */
        r1 = r1 + mem1;  /* This could be delay slot candidate */
        
        /* Conditional jump to label - should be simplejump_p */
        if (r1 > 50) {
            /* Jump target label with independent instruction */
            /* This is the 'next_trial' instruction after label */
            target1:
            /* Independent arithmetic - uses different registers */
            r2 = r2 & 0xFF;  /* Simple, non-trapping operation */
            /* Avoid jump, sequence, and resource conflicts */
            r3 = r3 - 1;
        } else {
            goto target1;
        }
        
        /* Pattern 2: Another conditional with different resources */
        r4 = r4 ^ 0x55;
        cond = mem2;
        
        if (cond != 0) {
            target2:
            /* Another independent operation after label */
            r2 = r2 + r3;  /* Uses different regs than delay slot candidate */
            r1 = r1 | 0x1;
        } else {
            goto target2;
        }
        
        /* Pattern 3: More complex to increase slots_to_fill analysis */
        mem3 = mem3 + 1;  /* Volatile store - can't be moved easily */
        
        if (r3 < 100) {
            target3:
            /* Simple integer operation that doesn't trap */
            r4 = (r4 * 2) & 0xFFFF;  /* Non-trapping multiplication */
            /* Ensure not a jump and no sequence */
            r2 = r2 >> 1;
        } else {
            goto target3;
        }
        
        /* Accumulate results to prevent dead code elimination */
        result += r1 + r2 + r3 + r4;
        
        /* Modify condition variables for branch variation */
        mem1 = mem1 + i;
        if (i % 3 == 0) {
            r1 = 1;  /* Reset to create new delay slot opportunities */
        }
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d\n", r1, r2, r3, r4);
    
    return 0;
}
