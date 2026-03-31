/* Target: MIPS with delay slots */
/* Compile with: -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg */
/* Alternative: -O3 -mips1 -fno-schedule-insns -fno-schedule-insns2 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile memory for non-optimizable resource patterns */
static volatile uint32_t mem_buffer[256];
static volatile uint32_t mem_result;

/* Register variables to control allocation */
register uint32_t r0 asm("t0");  /* Candidate delay slot reg */
register uint32_t r1 asm("t1");  /* Independent reg 1 */
register uint32_t r2 asm("t2");  /* Independent reg 2 */
register uint32_t r3 asm("t3");  /* Independent reg 3 */
register uint32_t r4 asm("t4");  /* Independent reg 4 */

NOMIPS16
int main(void) {
    uint32_t accumulator = 0;
    uint32_t loop_counter;
    
    /* Initialize volatile memory */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i * 3 + 1;
    }
    
    /* Initialize register variables */
    r0 = 0x12345678;
    r1 = 0x87654321;
    r2 = 0x55555555;
    r3 = 0xAAAAAAAA;
    r4 = 0x33333333;
    
    /* Main loop with multiple conditional jumps */
    for (loop_counter = 0; loop_counter < 1000; loop_counter++) {
        uint32_t temp;
        
        /* Pattern 1: Simple arithmetic as delay slot candidate */
        /* This could be scheduled into delay slot */
        r0 = r0 + 1;
        
        /* Conditional jump to label L1 */
        /* Must compile to simplejump_p with plain label target */
        if (r0 > 1000000) {
            goto L1;
        }
        
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than delay slot candidate (r0) */
        L1:
        r1 = r1 & 0xFFFF;  /* Simple arithmetic, non-trapping */
        
        /* Accumulate result to prevent elimination */
        accumulator += r1;
        
        
        /* Pattern 2: Another independent register set */
        r2 = r2 - 3;
        
        if (r2 < 500000) {
            goto L2;
        }
        
        L2:
        r3 = r3 | 0xFF00;  /* Different operation, different reg */
        accumulator += r3;
        
        
        /* Pattern 3: Memory operation as candidate */
        /* Volatile ensures resource analysis happens */
        temp = mem_buffer[(loop_counter & 0xFF)];
        
        if (temp > 200) {
            goto L3;
        }
        
        L3:
        r4 = r4 ^ 0x00FF;  /* Independent arithmetic after label */
        accumulator += r4;
        
        
        /* Pattern 4: More complex condition still simple jump */
        r0 = r0 * 2;
        
        if ((r0 & 0x1000) != 0 && (r1 & 0x2000) == 0) {
            goto L4;
        }
        
        L4:
        /* Independent load after label - different memory location */
        temp = mem_buffer[((loop_counter + 1) & 0xFF)];
        r2 = r2 + temp;  /* Uses different register than candidate */
        accumulator += r2;
        
        
        /* Pattern 5: Nested control flow to create multiple slots context */
        r3 = r3 >> 1;
        
        if (r3 != 0) {
            if (accumulator < 0x10000000) {
                goto L5;
            }
        }
        
        L5:
        /* Simple arithmetic with no resource conflicts */
        r1 = r1 + loop_counter;
        accumulator += r1;
        
        
        /* Pattern 6: Force reorg to consider multiple delay slots */
        /* by having consecutive conditional jumps */
        r4 = r4 << 1;
        
        if (r4 > r0) {
            goto L6a;
        }
        
        L6a:
        /* This instruction could be moved into delay slot */
        r0 = r0 + 100;
        
        if (r0 < r4) {
            goto L6b;
        }
        
        L6b:
        /* Independent instruction after second label */
        r2 = r2 - 50;
        accumulator += r2;
    }
    
    /* Final volatile store to ensure all operations complete */
    mem_result = accumulator;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    
    return 0;
}
