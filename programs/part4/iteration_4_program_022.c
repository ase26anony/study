/* Target: MIPS with delay slots - compile with: -O2 -march=mips1 */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int mem1 = 0, mem2 = 0, mem3 = 0;
    volatile int cond1 = 1, cond2 = 1, cond3 = 1;
    volatile int counter = 0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 100;  /* Candidate delay slot reg */
    register int r2 asm("t1") = 200;  /* Independent reg for next_trial */
    register int r3 asm("t2") = 300;  /* Another independent reg */
    register int r4 asm("t3") = 400;
    register int r5 asm("t4") = 500;
    register int r6 asm("t5") = 600;
    
    int result = 0;
    
    /* Loop to give reorg pass multiple opportunities */
    for (int i = 0; i < 1000; i++) {
        /* ----- First delay slot candidate scenario ----- */
        /* This arithmetic could be placed in delay slot */
        r1 = r1 + 1;  /* Sets resource in t0 */
        
        /* Conditional jump to label - should be simplejump_p */
        if (cond1 > 0) {
            /* Jump target label with independent instruction */
            /* This is the potential next_trial instruction */
            target1:
            /* Independent instruction: uses different register (t1) */
            r2 = r2 & 0xFF;  /* Simple arithmetic, no trap possible */
            result += r2;
        } else {
            goto target1;
        }
        
        /* ----- Second scenario with different resources ----- */
        /* Different register set for this candidate */
        r3 = r3 - mem1;  /* Uses volatile mem access */
        
        if (cond2 != 0) {
            target2:
            /* Independent: uses t4, not t3 */
            r4 = r4 | 0x55;  /* Non-trapping operation */
            result += r4;
        } else {
            goto target2;
        }
        
        /* ----- Third scenario with memory operations ----- */
        /* Store operation as delay slot candidate */
        mem2 = r5;
        
        if (counter++ < 500) {
            target3:
            /* Load from different memory location */
            r6 = mem3;  /* Independent memory access */
            result += r6;
        } else {
            goto target3;
        }
        
        /* ----- Fourth scenario: multiple jumps in sequence ----- */
        /* This increases slots_to_fill analysis opportunities */
        r1 = r1 * 2;
        
        if (cond3) {
            target4:
            r2 = r2 + r3;  /* Uses different regs than the jump's resources */
            result += r2;
            
            /* Another jump immediately after to create scheduling pressure */
            if (mem1 == 0) {
                target5:
                r4 = r4 >> 1;  /* Simple shift, no trap */
                result += r4;
            } else {
                goto target5;
            }
        } else {
            goto target4;
        }
        
        /* Modify conditions to vary control flow */
        cond1 = (i % 3) == 0;
        cond2 = (i % 5) == 0;
        cond3 = (i % 7) == 0;
        mem1 = i;
        mem3 = i * 2;
    }
    
    printf("Result: %d\n", result);
    printf("Final values: r1=%d, r2=%d, r3=%d, r4=%d, r5=%d, r6=%d\n", 
           r1, r2, r3, r4, r5, r6);
    
    return 0;
}
