/* Target: MIPS with delay slots - triggers specific uncovered reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create memory resource patterns */
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
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 100; i++) {
        /* BLOCK 1: Simple conditional jump with arithmetic before it */
        /* This arithmetic could be a delay slot candidate */
        r1 = r1 + mem1;  /* Uses t0, references mem1 */
        
        /* Simple conditional jump to label1 */
        /* Compiles to simplejump_p with plain label target */
        if (r1 > 50) {
            goto label1;
        }
        
        /* Some code to avoid fall-through optimization */
        r2 = r2 - mem2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (t2, t3) than the jump condition (t0) */
        r3 = r4 & 0xFF;  /* Simple bitwise AND, cannot trap */
        result += r3;
        
        
        /* BLOCK 2: Another conditional jump with different resources */
        r5 = r5 ^ mem3;  /* Uses t4, references mem3 */
        
        if (r5 != 0) {
            goto label2;
        }
        
        r6 = r6 | mem4;
        continue;
        
    label2:
        /* Another independent instruction - different register set */
        r2 = r3 + 1;  /* Uses t1, t2 - no overlap with r5 (t4) */
        result += r2;
        
        
        /* BLOCK 3: More complex to increase slots_to_fill analysis */
        /* Multiple jumps in sequence */
        r4 = r4 * 2;
        
        if (r4 < 1000) {
            goto label3;
        }
        
        r1 = r1 / 2;
        goto skip_label3;
        
    label3:
        /* Non-trapping arithmetic */
        r6 = r6 - 10;
        result += r6;
        
    skip_label3:
        
        /* BLOCK 4: Volatile access pattern */
        mem1 = mem1 + 1;
        r3 = mem2;
        
        if (r3 > 100) {
            goto label4;
        }
        
        mem3 = mem3 - 1;
        continue;
        
    label4:
        /* Simple arithmetic with constants only */
        r5 = 7 * 8;  /* Compile-time constant expression */
        result += r5;
        
        
        /* BLOCK 5: Register-only operations */
        r2 = r1 + r6;
        
        if (r2 != 0) {
            goto label5;
        }
        
        r3 = r4 - r5;
        continue;
        
    label5:
        /* Independent: uses registers not set by the jump's delay slot candidate */
        r4 = r4 & 0x0F;  /* r4 already used differently above */
        result += r4;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d\n", r1, r2, r3, r4, r5, r6);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
