/* Target: MIPS with delay slots - designed to trigger specific reorg.cc logic */
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
    register int r4 asm("t3") = 0;  /* Loop counter */
    
    int result = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (r4 = 0; r4 < 100; r4++) {
        /* Pattern 1: Simple conditional with arithmetic before jump */
        r1 = mem1 + r4;  /* This could be delay slot candidate */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r1 > 50) {
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = mem3 * 2;
        goto L2;
        
    L1:
        /* Instruction after label - potential next_trial */
        /* Uses different register (r2) than delay slot candidate (r1) */
        r2 = mem2 - 1;  /* Independent arithmetic - no resource conflict */
        result += r2;
        goto L3;
        
    L2:
        r3 = mem4 + 3;
        result += r3;
        
    L3:
        /* Pattern 2: Another conditional with different registers */
        r2 = mem2 * r4;  /* Different register for delay slot candidate */
        
        /* Conditional jump to label L4 */
        if (r2 < 2000) {
            goto L4;
        }
        
        r1 = mem1 / 2;
        goto L5;
        
    L4:
        /* Another independent instruction after label */
        r3 = mem3 & 0xFF;  /* Simple bitwise operation - non-trapping */
        result += r3;
        goto L6;
        
    L5:
        r1 = mem1 | 0x01;
        result += r1;
        
    L6:
        /* Pattern 3: Nested conditionals to create multiple jump opportunities */
        if (r4 % 3 == 0) {
            r1 = mem1 + 5;  /* Delay slot candidate */
            
            if (r1 < 100) {
                goto L7;
            }
            
            r2 = mem2 - 2;
            goto L8;
            
        L7:
            /* Independent arithmetic after label */
            r3 = mem3 ^ 0xAA;  /* XOR operation - simple, non-trapping */
            result += r3;
            goto L9;
            
        L8:
            r2 = mem2 * 3;
            result += r2;
            
        L9:
            /* Continue */
        }
        
        /* Pattern 4: Use volatile memory access to create specific resource patterns */
        mem1 = r4;  /* Volatile store - creates memory resource pattern */
        
        if (mem2 > 0) {  /* Volatile load in condition */
            r1 = mem3 + 1;  /* Delay slot candidate */
            goto L10;
        }
        
        r2 = mem4 - 1;
        goto L11;
        
    L10:
        /* Independent operation after label */
        r3 = r4 << 2;  /* Shift operation - simple, non-trapping */
        result += r3;
        goto L12;
        
    L11:
        r2 = r4 >> 1;
        result += r2;
        
    L12:
        /* Continue loop */
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print register values to ensure they're used */
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d\n", r1, r2, r3, r4);
    
    return 0;
}
