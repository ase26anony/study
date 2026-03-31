/* Target: MIPS with delay slots - triggers specific reorg.cc logic */
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
    register int r1 asm("t0") = 0;  /* Candidate delay slot register */
    register int r2 asm("t1") = 0;  /* Independent register for next_trial */
    register int r3 asm("t2") = 0;  /* Another independent register */
    register int r4 asm("t3") = 0;  /* Loop counter register */
    
    int result = 0;
    
    /* Loop to give reorg pass multiple opportunities */
    for (r4 = 0; r4 < 100; r4++) {
        /* Pattern 1: Simple arithmetic that could fill delay slot */
        r1 = mem1 + mem2;  /* This could be moved into delay slot */
        
        /* Conditional jump to label - must compile to simplejump_p */
        if (r1 > 0) {
            /* Target label with independent instruction */
            target1:
            /* Independent arithmetic - doesn't reference r1, uses different register */
            r2 = mem3 * mem4;  /* Potential next_trial */
            result += r2;
        }
        
        /* Pattern 2: Another independent pattern */
        r3 = mem2 - mem1;
        if (r3 != 0) {
            target2:
            /* Different operation, different registers */
            r2 = mem4 & mem3;  /* Bitwise operation - non-trapping */
            result ^= r2;
        }
        
        /* Pattern 3: More complex to increase analysis depth */
        r1 = mem1 << 2;
        if (r1 < 100) {
            target3:
            /* Simple arithmetic with constants - safe for delay slot */
            r2 = 5 + 3;  /* Compiler should see this as independent */
            result |= r2;
        }
        
        /* Pattern 4: Memory access pattern */
        mem1 = mem2 + 1;
        if (mem1 > 0) {
            target4:
            /* Register-only operation */
            r2 = r3 + 7;  /* Uses r3, not r1 */
            result -= r2;
        }
        
        /* Pattern 5: Nested control flow to create multiple slots_to_fill */
        r1 = mem3 * 2;
        if (r1 > 5) {
            /* Inner condition */
            if (mem4 < 10) {
                target5:
                /* Very simple independent operation */
                r2 = 1;  /* Just a constant assignment */
                result += r2;
            }
        }
        
        /* Modify volatiles to change conditions */
        mem1 = (mem1 + 1) & 0xF;
        mem2 = (mem2 + 2) & 0xF;
        mem3 = (mem3 + 3) & 0xF;
        mem4 = (mem4 + 4) & 0xF;
    }
    
    /* Use goto to ensure labels are referenced (prevent dead code elimination) */
    if (result < 0) goto target1;
    if (result > 1000) goto target2;
    if ((result & 1) == 0) goto target3;
    if ((result % 3) == 0) goto target4;
    if ((result % 5) == 0) goto target5;
    
    printf("Result: %d\n", result);
    return 0;
}
