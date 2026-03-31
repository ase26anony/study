/* Target: MIPS with delay slots - designed to trigger specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int mem1 = 0x1234;
    volatile int mem2 = 0x5678;
    volatile int mem3 = 0x9ABC;
    volatile int mem4 = 0xDEF0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");  /* Candidate delay slot register */
    register int r2 asm("t1");  /* Independent register for next_trial */
    register int r3 asm("t2");  /* Another independent register */
    register int r4 asm("t3");  /* Loop counter register */
    register int r5 asm("t4");  /* Accumulator register */
    
    int result = 0;
    
    /* Initialize registers */
    r1 = mem1;
    r2 = mem2;
    r3 = mem3;
    r4 = 100;  /* Loop iterations */
    r5 = 0;
    
    /* Main loop with multiple conditional jumps for delay slot analysis */
    while (r4-- > 0) {
        /* BLOCK 1: Simple conditional jump with arithmetic before it */
        /* This arithmetic could be a delay slot candidate */
        r1 = r1 + 1;  /* Candidate for delay slot filling */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r1 > 0x2000) {
            /* Jump target L1 immediately follows */
            goto L1;
        }
        
        /* Fall-through path */
        r3 = r3 & 0xFFF;
        goto after_L1;
        
    L1:
        /* Instruction after label - potential next_trial for delay slot */
        /* Uses different register (r2) than the candidate (r1) */
        r2 = r2 - 1;  /* Independent arithmetic - should pass resource checks */
        
    after_L1:
        /* Accumulate results to prevent elimination */
        r5 = r5 + r1 + r2;
        
        
        /* BLOCK 2: Another conditional jump pattern */
        /* Different arithmetic operation */
        r1 = r1 | 0x0100;
        
        /* Another simple conditional jump */
        if (r3 < 0x5000) {
            goto L2;
        }
        
        r2 = r2 ^ 0x00FF;
        goto after_L2;
        
    L2:
        /* Another independent instruction after label */
        r3 = r3 + 2;  /* Simple non-trapping arithmetic */
        
    after_L2:
        r5 = r5 + r3;
        
        
        /* BLOCK 3: Third pattern with memory access */
        /* Load operation as delay slot candidate */
        r1 = mem3;  /* Volatile load */
        
        /* Conditional jump based on different register */
        if (r2 != 0) {
            goto L3;
        }
        
        mem4 = r1;  /* Store in fall-through path */
        goto after_L3;
        
    L3:
        /* Independent arithmetic after label */
        r2 = r2 * 2;  /* Simple multiplication */
        
    after_L3:
        r5 = r5 + r2;
        
        
        /* BLOCK 4: Pattern with immediate values */
        r3 = 100;
        
        if (r5 > 5000) {
            goto L4;
        }
        
        r1 = r1 >> 1;
        goto after_L4;
        
    L4:
        /* Independent shift operation */
        r2 = r2 << 1;
        
    after_L4:
        r5 = r5 + (r1 & r2);
    }
    
    /* Use results to prevent dead code elimination */
    result = r5 + mem1 + mem2 + mem3 + mem4;
    
    printf("Result: %d\n", result);
    printf("Final values: r1=%d, r2=%d, r3=%d, r4=%d, r5=%d\n", r1, r2, r3, r4, r5);
    
    return result != 0 ? 0 : 1;
}
