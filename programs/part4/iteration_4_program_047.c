/* Target: MIPS with delay slots - triggers specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16, noinline))
static int check_condition(int x, int y) {
    return x > y;
}

int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int trigger = 1;
    volatile int counter = 0;
    volatile int mem1 = 0, mem2 = 0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 100;  /* Candidate delay slot register */
    register int r2 asm("t1") = 200;  /* Independent register for next_trial */
    register int r3 asm("t2") = 300;  /* Another independent register */
    register int r4 asm("t3") = 400;
    
    int result = 0;
    int i;
    
    /* Loop to give reorg pass multiple opportunities */
    for (i = 0; i < 100; i++) {
        /* BLOCK 1: First conditional jump with potential delay slot */
        /* This arithmetic could be placed in delay slot */
        r1 = r1 + trigger;  /* Uses t0 - delay slot candidate */
        
        /* Conditional jump to label1 - should be simplejump_p */
        if (check_condition(r1, 50)) {
            goto label1;
        }
        
        /* Fall-through path */
        r3 = r3 - 1;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (t1) to avoid resource conflicts */
        r2 = r2 & 0xFF;  /* Simple arithmetic, non-trapping */
        
        /* BLOCK 2: Second conditional jump chain */
        /* Another arithmetic operation */
        r4 = r4 ^ 0x55;
        
        if (check_condition(r2, 100)) {
            goto label2;
        }
        
        r3 = r3 + 2;
        continue;
        
    label2:
        /* Another independent instruction */
        r3 = r3 | 0xAA;
        
        /* BLOCK 3: Third with volatile memory access */
        /* Volatile access creates specific resource pattern */
        mem1 = r1 + r2;
        
        if (check_condition(r3, 200)) {
            goto label3;
        }
        
        r4 = r4 >> 1;
        continue;
        
    label3:
        /* Independent volatile operation */
        mem2 = r2 * 2;
        
        /* Accumulate results to prevent elimination */
        result += r1 + r2 + r3 + r4 + mem1 + mem2;
        counter++;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (iterations: %d)\n", result, counter);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d\n", r1, r2, r3, r4);
    
    return 0;
}
