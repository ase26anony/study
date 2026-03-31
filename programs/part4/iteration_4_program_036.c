/* Target: MIPS with delay slots - triggers reorg.cc lines 2135-2149 */
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
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int result = 0;
    int i;
    
    /* Initialize registers */
    r1 = 100;
    r2 = 200;
    r3 = 300;
    r4 = 400;
    r5 = 500;
    r6 = 600;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This could be a delay slot candidate (insn) */
        r1 = r1 + mem1;  /* Uses t0 and memory - sets resource pattern */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r1 > 50) {
            goto L1;
        }
        
        /* Fall-through path */
        r2 = r2 - mem2;
        continue;
        
    L1:
        /* Instruction after label (next_trial) - independent of r1 */
        /* Uses different register (t2) and different memory location */
        r3 = r3 & mem3;  /* Simple integer arithmetic, non-trapping */
        
        /* Accumulate result to prevent elimination */
        result += r3;
        
        
        /* Pattern 2: Another conditional jump pattern */
        /* Different register set for delay slot candidate */
        r4 = r4 ^ mem4;  /* XOR operation - sets resource pattern */
        
        if (r4 != 0) {
            goto L2;
        }
        
        r5 = r5 | 0xFF;
        continue;
        
    L2:
        /* Independent instruction after label */
        r6 = r6 + 1;  /* Simple increment, uses t5 */
        result += r6;
        
        
        /* Pattern 3: Nested conditional for multiple slot analysis */
        r2 = r2 * 2;  /* Multiplication - may create interesting pattern */
        
        if (r2 < 1000) {
            goto L3;
        }
        
        r1 = r1 / 2;
        continue;
        
    L3:
        /* Independent arithmetic after label */
        r5 = r5 - 2;  /* Different register from the multiplication */
        result += r5;
        
        
        /* Pattern 4: Memory access pattern */
        /* Volatile load as potential delay slot candidate */
        int temp = mem1;  /* Load from volatile */
        
        if (temp > 0) {
            goto L4;
        }
        
        mem2 = temp;
        continue;
        
    L4:
        /* Independent store after label */
        mem3 = r3;  /* Store to different memory location */
        result += mem3;
        
        
        /* Pattern 5: Register pressure test */
        /* Swap registers to create resource patterns */
        int tmp_reg = r1;
        r1 = r2;
        r2 = tmp_reg;
        
        if (r1 > r2) {
            goto L5;
        }
        
        r3 = r3 + r4;
        continue;
        
    L5:
        /* Simple independent operation */
        r6 = r6 ^ r6;  /* Clear register - cannot trap */
        result += 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r1, r2, r3, r4, r5, r6);
    printf("Memory: %d, %d, %d, %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
