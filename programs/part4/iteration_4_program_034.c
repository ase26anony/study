/* Target: MIPS with delay slot filling */
/* Compile with: -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2 */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create memory ops */
    volatile int mem1 = 100;
    volatile int mem2 = 200;
    volatile int mem3 = 300;
    volatile int mem4 = 400;
    
    /* Register variables to control register allocation */
    register int r0 asm("t0") = 0;  /* Candidate delay slot reg */
    register int r1 asm("t1") = 1;  /* Independent reg for next_trial */
    register int r2 asm("t2") = 2;  /* Another independent reg */
    register int r3 asm("t3") = 3;  /* Yet another independent reg */
    register int r4 asm("t4") = 4;  /* Loop counter */
    register int r5 asm("t5") = 5;  /* Accumulator */
    register int r6 asm("t6") = 6;  /* Temp */
    register int r7 asm("t7") = 7;  /* Temp */
    
    int result = 0;
    
    /* Loop to give reorg pass multiple opportunities */
    for (r4 = 0; r4 < 100; r4++) {
        /* BLOCK 1: Simple arithmetic that could fill delay slot */
        r0 = mem1 + r4;  /* Uses r0 and memory - delay slot candidate */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r0 > 50) {
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r5 = r5 + 1;
        continue;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) and different memory location */
        r1 = mem2 & 0xFF;  /* Simple, non-trapping operation */
        
        /* BLOCK 2: Another pattern with different registers */
        r2 = mem3 * 2;  /* Another delay slot candidate */
        
        if (r2 < 400) {
            goto L2;
        }
        
        r6 = r6 - 1;
        continue;
        
    L2:
        /* Another independent instruction */
        r3 = mem4 | 0x01;  /* Simple bitwise operation */
        
        /* BLOCK 3: Pattern with volatile store */
        mem1 = r0 + r1;  /* Memory op as delay slot candidate */
        
        if (r3 != 0) {
            goto L3;
        }
        
        r7 = r7 ^ 1;
        continue;
        
    L3:
        /* Independent load after label */
        r6 = mem2;  /* Simple load */
        
        /* Accumulate results to prevent elimination */
        result += r0 + r1 + r2 + r3 + r5 + r6 + r7;
        
        /* BLOCK 4: Pattern with register-only operations */
        r5 = r4 << 2;  /* Shift operation */
        
        if (r5 > 100) {
            goto L4;
        }
        
        r0 = r0 + 1;
        continue;
        
    L4:
        /* Independent arithmetic */
        r7 = r1 + r2;  /* Uses different registers than the jump condition */
        
        /* BLOCK 5: Final pattern with complex condition */
        r6 = mem3 - mem4;
        
        /* Multiple conditions to ensure simplejump_p */
        if (r6 > -100 && r6 < 100) {
            goto L5;
        }
        
        r2 = r2 * 2;
        continue;
        
    L5:
        /* Final independent instruction */
        r3 = r4 % 16;  /* Simple modulo - non-trapping with power of 2 */
        
        /* More accumulation */
        result += r5 + r6 + r7;
    }
    
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d, t6=%d, t7=%d\n",
           r0, r1, r2, r3, r4, r5, r6, r7);
    
    return 0;
}
