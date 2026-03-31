/* Target: MIPS with delay slots - triggers specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int mem1 = 1, mem2 = 2, mem3 = 3, mem4 = 4;
    volatile int result = 0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    register int r7 asm("t6");
    register int r8 asm("t7");
    
    /* Initialize registers with distinct values */
    r1 = 100; r2 = 200; r3 = 300; r4 = 400;
    r5 = 500; r6 = 600; r7 = 700; r8 = 800;
    
    int i;
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This could be a delay slot candidate (insn) */
        r1 = r1 + mem1;  /* Uses r1, mem1 - independent from target instructions */
        
        if (r1 > 50) {
            /* Jump to label with simplejump_p */
            goto label1;
        }
        
        /* Fall-through path */
        r2 = r2 - mem2;
        continue;
        
    label1:
        /* Instruction after label (next_trial) - independent from r1/mem1 */
        /* Uses different registers: r3, r4, mem3 */
        r3 = r4 + mem3;  /* Simple arithmetic, non-trapping */
        result += r3;
        
        /* Pattern 2: Another conditional jump with different registers */
        r5 = r5 & 0xFF;  /* Bitwise operation - delay slot candidate */
        
        if (r5 != 0) {
            goto label2;
        }
        
        r6 = r6 | 0x55;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r7 = r8 - mem4;  /* Different registers from r5 */
        result += r7;
        
        /* Pattern 3: Nested conditional to create more opportunities */
        r2 = r2 + 1;
        
        if (r2 < 1000) {
            goto label3;
        }
        
        r3 = r3 - 1;
        continue;
        
    label3:
        /* Simple arithmetic that doesn't reference r2's resources */
        r4 = mem1 * 2;  /* Uses mem1 but not r2 - check resource analysis */
        result += r4;
        
        /* Pattern 4: More complex condition but still simple jump */
        r6 = r6 ^ 0xAA;
        
        if (r6 > 100 && r6 < 200) {
            goto label4;
        }
        
        r7 = r7 + 5;
        continue;
        
    label4:
        /* Independent load-like operation */
        r8 = mem2 + 10;  /* Different from r6's resources */
        result += r8;
        
        /* Pattern 5: Loop counter check with jump */
        if (i % 3 == 0) {
            goto label5;
        }
        
        r1 = r1 * 2;
        continue;
        
    label5:
        /* Final independent operation */
        r5 = r3 + r4;  /* Uses r3,r4 but not i or r1 */
        result += r5;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8);
    
    return 0;
}
