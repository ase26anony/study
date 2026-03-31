/* Target: MIPS with delay slots - triggers specific uncovered lines in reorg.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NO_MIPS16 __attribute__((nomips16))

/* Volatile memory for resource conflict analysis */
volatile int g_mem1 = 100;
volatile int g_mem2 = 200;
volatile int g_mem3 = 300;

NO_MIPS16
int main(void) {
    /* Explicit register variables to control resource allocation */
    register int r0 asm("t0");  /* Candidate delay slot register */
    register int r1 asm("t1");  /* Independent register for next_trial */
    register int r2 asm("t2");  /* Loop counter */
    register int r3 asm("t3");  /* Accumulator */
    register int r4 asm("t4");  /* Temporary */
    register int r5 asm("t5");  /* Another independent register */
    
    int result = 0;
    
    /* Initialize registers */
    r0 = 1;
    r1 = 2;
    r2 = 0;
    r3 = 0;
    r4 = 0;
    r5 = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (r2 = 0; r2 < 100; r2++) {
        /* Pattern 1: Simple conditional with arithmetic before jump */
        /* This creates a delay slot candidate (insn) */
        r0 = r0 + r2;  /* Candidate for delay slot - uses r0 */
        
        /* Conditional jump to label L1 */
        if (r0 > 50) {
            /* Jump target L1 - next_trial should be independent */
            goto L1;
        }
        
        /* Fall-through path */
        r3 = r3 + 1;
        continue;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than the delay slot candidate (r0) */
        r1 = r1 * 2;  /* This should be eligible for moving into delay slot */
        
        /* Continue with other operations */
        r3 = r3 + r1;
        
        
        /* Pattern 2: Another conditional with different resources */
        /* Use volatile memory to create specific resource patterns */
        r4 = g_mem1;  /* Load from volatile - creates memory resource */
        
        if (r4 < 150) {
            goto L2;
        }
        
        r3 = r3 - 1;
        continue;
        
    L2:
        /* Independent arithmetic using different register */
        r5 = r5 & 0xFF;  /* Simple non-trapping operation */
        
        /* Use result to prevent elimination */
        result += r5;
        
        
        /* Pattern 3: Nested conditionals for multiple slot analysis */
        r0 = r0 ^ r2;  /* Bitwise operation - candidate */
        
        if (r0 != 0) {
            if (r3 > 10) {
                goto L3;
            }
        }
        
        r3 = r3 + 2;
        continue;
        
    L3:
        /* Another independent instruction */
        r1 = r1 + 3;  /* Simple addition */
        
        /* Access different volatile memory location */
        g_mem2 = r1;  /* Store to volatile */
        
        
        /* Pattern 4: Complex enough to avoid becoming a trap */
        r4 = g_mem3;  /* Load candidate */
        
        /* Simple condition for simplejump_p */
        if (r4 == 300) {
            goto L4;
        }
        
        r3 = r3 * 2;
        continue;
        
    L4:
        /* Independent operation with no resource conflicts */
        r5 = r5 | 0x55;  /* Bitwise OR - cannot trap */
        
        /* Force register usage to prevent optimization */
        asm volatile("" : "+r" (r5));
    }
    
    /* Use all results to prevent dead code elimination */
    result += r0 + r1 + r2 + r3 + r4 + r5;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    return 0;
}
