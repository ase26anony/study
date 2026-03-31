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
    register int r0 asm("t0") = 0;  /* Candidate delay slot register */
    register int r1 asm("t1") = 0;  /* Independent register for next_trial */
    register int r2 asm("t2") = 0;  /* Another independent register */
    register int r3 asm("t3") = 0;  /* Loop counter register */
    register int r4 asm("t4") = 0;  /* Accumulator register */
    
    int result = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (r3 = 0; r3 < 100; r3++) {
        /* First conditional jump block - designed for delay slot filling */
        r0 = mem1 + 5;  /* This could be a delay slot candidate */
        
        /* Simple conditional jump that should become simplejump_p */
        if (r0 > 0) {
            /* Jump to label with independent instruction after it */
            goto label1;
        }
        
        /* Fall-through path */
        r4 += mem2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than delay slot candidate (r0) */
        r1 = mem3 & 0xFF;  /* Simple non-trapping operation */
        r4 += r1;
        
        /* Second conditional jump block - another opportunity */
        r2 = mem4 - 1;  /* Another delay slot candidate */
        
        if (r2 != 0) {
            goto label2;
        }
        
        r4 += mem1;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r1 = r1 | 0x01;  /* Simple bitwise operation, non-trapping */
        r4 += r1;
        
        /* Third conditional jump block - tight sequence for multiple slots analysis */
        r0 = mem2 * 2;  /* Delay slot candidate */
        
        if (mem1 < mem3) {  /* Simple comparison */
            goto label3;
        }
        
        r4 += mem4;
        continue;
        
    label3:
        /* Independent arithmetic after label */
        r1 = r1 + 1;  /* Simple addition */
        r4 += r1;
        
        /* Use results to prevent dead code elimination */
        result += r0 + r1 + r2 + r4;
        
        /* Memory operations with volatile to create resource patterns */
        mem1 = r0;
        mem2 = r1;
        mem3 = r2;
        mem4 = r4 & 0xFF;
    }
    
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d\n", r0, r1, r2, r3, r4);
    printf("Memory: %d, %d, %d, %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
