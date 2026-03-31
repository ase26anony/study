/* Target: MIPS with delay slot filling */
/* Compile with: -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create memory dependencies */
    volatile int mem1 = 100;
    volatile int mem2 = 200;
    volatile int mem3 = 300;
    volatile int mem4 = 400;
    
    /* Register variables to control allocation and avoid conflicts */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    register int r7 asm("t6");
    register int r8 asm("t7");
    
    int result = 0;
    int i;
    
    /* Initialize registers */
    r1 = 1; r2 = 2; r3 = 3; r4 = 4;
    r5 = 5; r6 = 6; r7 = 7; r8 = 8;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with independent instruction after label */
        /* This creates a candidate for delay slot filling */
        r1 = r1 + mem1;  /* Potential delay slot instruction */
        if (r1 > 0) {    /* simplejump_p conditional */
            /* Jump to label with independent instruction */
            goto label1;
        }
        /* Fall-through path */
        r2 = r2 - 1;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers than the delay slot candidate */
        r3 = r3 & 0xFF;  /* Simple non-trapping arithmetic */
        result += r3;
        
        /* Pattern 2: Another conditional jump chain */
        r4 = r4 * 2;     /* Another delay slot candidate */
        if (r4 != 0) {   /* Another simplejump_p */
            goto label2;
        }
        r5 = r5 + 1;
        continue;
        
    label2:
        /* Another independent instruction */
        r6 = r6 | 0x55;  /* Simple bitwise operation */
        result += r6;
        
        /* Pattern 3: More complex resource separation */
        /* Use volatile memory access in delay slot candidate */
        mem1 = mem1 + 1;  /* Volatile store - creates specific resource pattern */
        if (mem2 > 150) { /* Conditional using different volatile */
            goto label3;
        }
        mem3 = mem3 - 1;
        continue;
        
    label3:
        /* Instruction that doesn't conflict with the volatile store */
        r7 = r7 ^ r8;    /* Uses only register resources */
        result += r7;
        
        /* Pattern 4: Multiple jumps in sequence */
        r2 = r2 << 1;    /* Shift operation */
        if (r2 < 1000) {
            goto label4;
        }
        r1 = r1 >> 1;
        continue;
        
    label4:
        /* Simple arithmetic with constants */
        r8 = r8 + 42;    /* Constant addition - won't trap */
        result += r8;
        
        /* Pattern 5: Avoid SEQUENCE creation */
        /* Use simple operations that won't be expanded */
        r5 = r5 - r6;
        if (r5 != 0) {
            goto label5;
        }
        r4 = r4 + r3;
        continue;
        
    label5:
        /* Final independent instruction */
        r1 = (r1 * 3) / 2;  /* More complex but still safe */
        result += r1;
        
        /* Loop control - prevents infinite loops */
        if (i % 100 == 0) {
            mem4 = result;  /* Volatile store to prevent elimination */
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
