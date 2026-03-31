/* reorg_delay_slot_test.c
 * Target: MIPS with delay slots
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 * Or for coverage: mips-linux-gnu-gcc -O2 -march=mips1 -fsanitize=address -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int32_t mem1 = 100;
    volatile int32_t mem2 = 200;
    volatile int32_t mem3 = 300;
    volatile int32_t mem4 = 400;
    
    /* Explicit register variables to control resource allocation */
    register int32_t r0 asm("t0") = 0;  /* Candidate delay slot register */
    register int32_t r1 asm("t1") = 1;  /* Independent register for next_trial */
    register int32_t r2 asm("t2") = 2;  /* Another independent register */
    register int32_t r3 asm("t3") = 3;
    register int32_t r4 asm("t4") = 4;
    register int32_t r5 asm("t5") = 5;
    
    int32_t accumulator = 0;
    int32_t loop_counter;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (loop_counter = 0; loop_counter < 100; loop_counter++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        r0 = mem1 + loop_counter;  /* Candidate for delay slot filling */
        if (r0 > 50) {
            /* Jump to label with independent instruction after */
            goto label1;
        }
        /* Fall-through path */
        r2 = r2 * 2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than delay slot candidate (r0) */
        r1 = mem2 - r3;
        accumulator += r1;
        
        /* Pattern 2: Another conditional jump with different registers */
        r4 = mem3 ^ loop_counter;  /* Another delay slot candidate */
        if (r4 != 0) {
            goto label2;
        }
        r5 = r5 + 1;
        continue;
        
    label2:
        /* Another independent instruction - different resource set */
        r3 = r1 & 0xFF;  /* Uses r1, not r4 */
        accumulator += r3;
        
        /* Pattern 3: More complex to increase slots_to_fill analysis */
        r2 = mem4 | loop_counter;
        if (r2 < 1000) {
            goto label3;
        }
        r0 = r0 - 1;
        continue;
        
    label3:
        /* Simple arithmetic that doesn't reference r2's resources */
        r5 = r5 << 2;  /* Shift uses different register */
        accumulator += r5;
        
        /* Pattern 4: Nested condition to create multiple jump opportunities */
        r1 = mem1 * 2;
        if (r1 > 150) {
            if (accumulator < 5000) {
                goto label4;
            }
        }
        r3 = r3 / 2;
        continue;
        
    label4:
        /* Safe arithmetic - no division or trapping operations */
        r4 = r4 + 100;
        accumulator += r4;
        
        /* Pattern 5: Volatile memory access as delay slot candidate */
        mem1 = mem1 + 1;  /* Volatile store */
        if (mem2 > 100) {
            goto label5;
        }
        mem3 = mem3 - 1;
        continue;
        
    label5:
        /* Independent volatile load */
        r0 = mem4;  /* Load from different memory location */
        accumulator += r0;
        
        /* Pattern 6: Mixed operations to test resource analysis */
        r3 = r1 + r2;  /* Uses two registers */
        if (r3 != 0) {
            goto label6;
        }
        r4 = r4 ^ r5;
        continue;
        
    label6:
        /* Completely independent operation */
        r5 = 0x1234;  /* Constant assignment */
        accumulator += r5;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Accumulator: %d\n", accumulator);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r0, r1, r2, r3, r4, r5);
    printf("Memory: mem1=%d, mem2=%d, mem3=%d, mem4=%d\n",
           mem1, mem2, mem3, mem4);
    
    return accumulator != 0 ? 0 : 1;
}
