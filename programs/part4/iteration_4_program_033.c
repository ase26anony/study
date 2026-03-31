/* Target: MIPS with delay slot filling */
/* Compile with: -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create memory dependencies */
    volatile int32_t mem1 = 100;
    volatile int32_t mem2 = 200;
    volatile int32_t mem3 = 300;
    volatile int32_t mem4 = 400;
    
    /* Explicit register variables to control resource allocation */
    register int32_t r1 asm("t0");
    register int32_t r2 asm("t1");
    register int32_t r3 asm("t2");
    register int32_t r4 asm("t3");
    register int32_t r5 asm("t4");
    register int32_t r6 asm("t5");
    register int32_t r7 asm("t6");
    register int32_t r8 asm("t7");
    
    int32_t accumulator = 0;
    int32_t loop_counter;
    
    /* Initialize register variables with distinct values */
    r1 = mem1;
    r2 = mem2;
    r3 = mem3;
    r4 = mem4;
    r5 = 500;
    r6 = 600;
    r7 = 700;
    r8 = 800;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (loop_counter = 0; loop_counter < 100; loop_counter++) {
        /* First conditional jump block - designed for delay slot filling */
        /* This creates a simplejump_p to label1 */
        if (r1 > r2) {
            /* This arithmetic could be in delay slot */
            r1 = r1 + r3;
            goto label1;
        }
        
        /* Independent arithmetic before label - potential next_trial */
        r4 = r4 & 0xFF;  /* Simple non-trapping operation */
        
    label1:
        /* Instruction after label - must be eligible for moving into delay slot */
        /* Uses different registers than the jump block to avoid resource conflicts */
        r5 = r6 + r7;  /* Independent arithmetic - doesn't reference r1, r2, r3 */
        accumulator += r5;
        
        /* Second conditional jump block - different registers */
        /* Creates another simplejump_p to label2 */
        if (r8 < 1000) {
            /* Another delay slot candidate */
            r8 = r8 - 50;
            goto label2;
        }
        
        /* More independent arithmetic */
        r2 = r2 | 0x55;
        
    label2:
        /* Another eligible instruction after label */
        r3 = r4 ^ r5;  /* Uses different register set */
        accumulator += r3;
        
        /* Third conditional jump with memory operations */
        if (mem1 != mem2) {
            /* Volatile load - creates specific resource pattern */
            int32_t temp = mem3;
            r6 = temp * 2;
            goto label3;
        }
        
        /* Arithmetic that doesn't trap */
        r7 = r7 >> 1;
        
    label3:
        /* Simple arithmetic after label - eligible for delay slot */
        r1 = r2 + 10;  /* Different from jump's registers */
        accumulator += r1;
        
        /* Fourth conditional jump - complex enough to require reorg analysis */
        if ((r1 + r2) > (r3 + r4)) {
            /* Multiple operations - some might be delay slot candidates */
            r5 = r5 - r6;
            r7 = r7 + 1;
            goto label4;
        }
        
        /* Non-trapping operation */
        r8 = r8 & ~0x01;
        
    label4:
        /* Final eligible instruction in the sequence */
        r4 = r3 * 2;  /* Simple multiplication */
        accumulator += r4;
        
        /* Modify values to change branch behavior */
        mem1 += 1;
        mem2 += 2;
        r1 += loop_counter;
        r2 -= loop_counter;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Accumulator: %d\n", accumulator);
    printf("Registers: %d %d %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return accumulator != 0 ? 0 : 1;
}
