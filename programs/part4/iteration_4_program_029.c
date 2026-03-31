#include <stdio.h>

/* Prevent MIPS16 mode which may not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 0x1234;
    volatile int mem2 = 0x5678;
    volatile int mem3 = 0x9ABC;
    volatile int mem4 = 0xDEF0;
    
    /* Register variables to control register allocation */
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
    
    /* Initialize registers with distinct values */
    r1 = 100;
    r2 = 200;
    r3 = 300;
    r4 = 400;
    r5 = 500;
    r6 = 600;
    r7 = 700;
    r8 = 800;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This creates a delay slot candidate (r1 = r1 + mem1) */
        r1 = r1 + mem1;  /* Potential delay slot instruction */
        
        /* Conditional jump to label1 - should be simplejump_p */
        if (r1 > 0) {
            goto label1;
        }
        
        /* Fall-through path */
        r2 = r2 - mem2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (r3, r4) than the delay slot candidate */
        r3 = r3 & r4;    /* Simple integer arithmetic, non-trapping */
        result += r3;
        
        /* Pattern 2: Another conditional jump with different registers */
        r5 = r5 ^ mem3;  /* Another delay slot candidate */
        
        if (r5 != 0) {
            goto label2;
        }
        
        r6 = r6 | mem4;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r7 = r7 << 2;    /* Simple shift operation */
        result += r7;
        
        /* Pattern 3: Third conditional jump to create multiple opportunities */
        r8 = r8 + i;     /* Delay slot candidate with loop variable */
        
        if (r8 < 10000) {
            goto label3;
        }
        
        r1 = r1 * 2;
        continue;
        
    label3:
        /* Independent arithmetic after label */
        r2 = r2 >> 1;    /* Simple shift, different register from candidate */
        result += r2;
        
        /* Pattern 4: More complex to increase slots_to_fill analysis */
        /* Use volatile memory access as delay slot candidate */
        mem1 = mem1 + 1;
        
        if (mem1 > 5000) {
            goto label4;
        }
        
        mem2 = mem2 - 1;
        continue;
        
    label4:
        /* Independent volatile operation after label */
        mem3 = mem3 & 0xFF;
        result += mem3;
        
        /* Pattern 5: Nested conditional to create control flow complexity */
        r3 = r3 + r4;
        
        if (r3 > r4) {
            if (r4 < 1000) {
                goto label5;
            }
        }
        
        r5 = r5 - r6;
        continue;
        
    label5:
        /* Simple arithmetic after label */
        r6 = r6 * 3;
        result += r6;
        
        /* Pattern 6: Use different condition types */
        r7 = r7 % 17;
        
        if (r7 == 0) {
            goto label6;
        }
        
        r8 = r8 / 2;
        continue;
        
    label6:
        /* Final independent operation */
        r1 = r1 & 0xFFFF;
        result += r1;
    }
    
    /* Use all results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d %d %d\n", r1, r2, r3, r4, r5, r6, r7, r8);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
