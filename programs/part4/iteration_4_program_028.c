/* reorg_coverage.c - Target MIPS delay slot filling for uncovered reorg.cc lines */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile memory for resource conflict analysis */
volatile int mem1, mem2, mem3, mem4;

/* Register variables to control allocation */
register int r0 asm("t0");
register int r1 asm("t1");
register int r2 asm("t2");
register int r3 asm("t3");
register int r4 asm("t4");
register int r5 asm("t5");

NOMIPS16 int main(void) {
    int i, result = 0;
    
    /* Initialize register variables with distinct values */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5; r5 = 6;
    
    /* Initialize volatile memory */
    mem1 = 100; mem2 = 200; mem3 = 300; mem4 = 400;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple arithmetic before conditional jump */
        /* This could be the delay slot candidate (insn) */
        r0 = r1 + r2;  /* Uses t0, t1, t2 */
        
        /* Conditional jump to label L1 */
        if (r0 > 0) {
            /* Jump target L1 - next_trial candidate must be here */
            goto L1;
        }
        
        /* Fall-through path */
        r3 = r4 - r5;
        continue;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (t3, t4) than the jump condition */
        r3 = r4 & r5;  /* Simple non-trapping arithmetic */
        
        /* Another pattern with memory operations */
        mem1 = mem2 + 1;  /* Volatile store - creates resource pattern */
        
        if (mem3 != 0) {
            goto L2;
        }
        
        r0 = r1 | r2;
        continue;
        
    L2:
        /* Another independent instruction after label */
        /* Uses completely different resource set */
        r5 = r0 ^ 0xFF;  /* XOR operation, cannot trap */
        
        /* Pattern 3: Nested control flow */
        r2 = r3 * 2;  /* Multiplication candidate */
        
        if (r2 < 1000) {
            goto L3;
        }
        
        mem4 = mem3;
        continue;
        
    L3:
        /* Simple arithmetic with no resource conflicts */
        r4 = r5 + 1;  /* Different register set than r2=r3*2 */
        
        /* Pattern 4: Multiple consecutive jumps */
        r1 = r0 - r5;
        
        if (r1 != 0) {
            goto L4;
        }
        
        r2 = r3 + r4;
        continue;
        
    L4:
        /* Instruction that doesn't reference previous resources */
        /* Uses volatile memory which creates specific resource patterns */
        int temp = mem1;  /* Load from volatile */
        r0 = temp & 0x0F; /* Mask operation */
        
        /* Accumulate results to prevent elimination */
        result += r0 + r1 + r2 + r3 + r4 + r5;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Memory values: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
