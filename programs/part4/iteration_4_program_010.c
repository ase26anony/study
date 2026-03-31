/* 
 * MIPS delay slot filling test case targeting specific uncovered lines in reorg.cc
 * Compile with: -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 1;
    volatile int mem2 = 2;
    volatile int mem3 = 3;
    volatile int mem4 = 4;
    
    /* Explicit register variables to control resource allocation */
    register int r0 asm("t0") = 0;  /* Candidate delay slot register */
    register int r1 asm("t1") = 0;  /* Independent register for next_trial */
    register int r2 asm("t2") = 0;  /* Another independent register */
    register int r3 asm("t3") = 0;  /* Condition testing register */
    register int r4 asm("t4") = 0;  /* Accumulator */
    
    int i;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        r3 = mem1 + i;
        
        /* This arithmetic could be a delay slot candidate */
        r0 = r3 * 2;  /* Uses r0 (t0) */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r3 > 50) {
            goto L1;
        }
        
        /* Fall-through path */
        r4 += r0;
        continue;
        
    L1:
        /* Instruction after label - potential next_trial */
        /* Uses different register (r1/t1) to avoid resource conflicts */
        r1 = mem2 + 1;  /* Simple arithmetic, non-trapping */
        r4 += r1;
        
        
        /* Pattern 2: Another conditional with different registers */
        r3 = mem3 - i;
        
        /* Another delay slot candidate using r2 (t2) */
        r2 = r3 & 0xFF;
        
        if (r3 < 0) {
            goto L2;
        }
        
        r4 += r2;
        continue;
        
    L2:
        /* Another independent next_trial candidate */
        /* Uses r1 again, but r2 was used in delay slot candidate - no conflict */
        r1 = mem4 * 2;
        r4 += r1;
        
        
        /* Pattern 3: Nested conditions to create multiple jump opportunities */
        r3 = i * 3;
        
        /* Delay slot candidate using memory (volatile ensures resource analysis) */
        mem1 = r3 + 5;
        
        if (r3 % 2 == 0) {
            goto L3;
        }
        
        r4 += mem1;
        continue;
        
    L3:
        /* next_trial with register that doesn't conflict with mem1 access */
        r0 = r3 >> 1;
        r4 += r0;
        
        
        /* Pattern 4: More complex but still simple jump */
        r3 = mem2 + mem3;
        
        /* Multiple arithmetic ops - compiler might choose one for delay slot */
        r0 = r3 + 1;
        r2 = r0 - 2;
        
        if (r3 != 0) {
            goto L4;
        }
        
        r4 += r2;
        continue;
        
    L4:
        /* Independent arithmetic after label */
        r1 = mem4 ^ 0x55;  /* Bitwise operation, non-trapping */
        r4 += r1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (r0=%d, r1=%d, r2=%d, r3=%d, r4=%d)\n", 
           r4, r0, r1, r2, r3, r4);
    
    /* Also use volatile memory to ensure all operations are preserved */
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return r4 > 0 ? 0 : 1;
}
