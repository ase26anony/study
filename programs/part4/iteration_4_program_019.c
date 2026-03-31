/* Target: MIPS with delay slots - designed to trigger reorg.cc lines 2135-2149 */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
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
    
    /* Loop to increase reorg pass analysis opportunities */
    for (int i = 0; i < 100; i++) {
        /* BLOCK 1: Simple conditional jump with arithmetic in delay slot position */
        r0 = mem1 + 10;  /* This could be a delay slot candidate */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r2 > 0) {
            /* Force label placement with goto */
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = r3 + mem2;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (r1) than the delay slot candidate (r0) */
        r1 = r4 & 0xFF;  /* Simple non-trapping arithmetic */
        result += r1;
        
        /* BLOCK 2: Another conditional jump pattern */
        r0 = mem3 * 2;  /* Another delay slot candidate */
        
        if (r5 < 100) {
            goto L2;
        }
        
        r3 = r3 - mem4;
        
    L2:
        /* Another independent instruction - different resource pattern */
        r4 = r5 | 0x0F;  /* Simple bitwise operation */
        result += r4;
        
        /* BLOCK 3: Third pattern with memory access */
        /* Volatile load creates specific resource pattern */
        r0 = mem1;  /* Delay slot candidate with memory reference */
        
        if (r3 != 0) {
            goto L3;
        }
        
        r2 = r2 + 1;
        
    L3:
        /* Independent store operation */
        r5 = r1 + r2;  /* Uses different registers than r0 */
        result += r5;
        
        /* BLOCK 4: Pattern with immediate values */
        r0 = 42;  /* Simple immediate assignment */
        
        if (r4 > r1) {
            goto L4;
        }
        
        r3 = r3 * 2;
        
    L4:
        /* Shift operation - independent of r0 */
        r1 = r2 << 2;  /* Simple shift */
        result += r1;
        
        /* Modify condition variables to create varying branch behavior */
        r2 = (r2 + 1) & 0x7;
        r5 = r5 + 1;
    }
    
    /* Use all results to prevent dead code elimination */
    printf("Result: %d (r0=%d, r1=%d, r2=%d, r3=%d, r4=%d, r5=%d)\n", 
           result, r0, r1, r2, r3, r4, r5);
    
    return result != 0 ? 0 : 1;
}
