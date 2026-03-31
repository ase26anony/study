/* Target: MIPS with delay slots - compile with: -O2 -march=mips1 */
#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define MIPS_NODELAY __attribute__((nomips16, noinline))

/* Volatile memory for resource conflict control */
static volatile uint32_t mem_buffer[256];
static volatile uint32_t mem_result = 0;

MIPS_NODELAY
int main(void) {
    /* Explicit register variables to control resource allocation */
    register uint32_t r1 asm("t0");  /* Candidate delay slot reg */
    register uint32_t r2 asm("t1");  /* Independent reg for next_trial */
    register uint32_t r3 asm("t2");  /* Condition variable */
    register uint32_t r4 asm("t3");  /* Loop counter */
    register uint32_t r5 asm("t4");  /* Accumulator */
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
    }
    
    r4 = 0;     /* Loop counter */
    r5 = 0;     /* Result accumulator */
    
    /* Main loop to give reorg pass multiple opportunities */
    while (r4 < 100) {
        /* Setup condition for conditional jumps */
        r3 = r4 & 0xF;  /* Simple condition variable */
        
        /* ----- PATTERN 1: Simple conditional jump with independent ops ----- */
        /* This creates a candidate for delay slot filling */
        r1 = mem_buffer[r4 & 0xFF];  /* Load - could be in delay slot */
        
        /* Conditional jump to label L1 */
        if (r3 > 7) {
            /* Target label must be a plain label for jump_to_label_p */
            goto L1;
        }
        
        /* Fall-through path */
        r5 += r1 * 2;
        goto L2;
        
    L1:
        /* Instruction after label (potential next_trial) */
        /* Must be: NONJUMP_INSN_P, not SEQUENCE, not JUMP_P */
        /* Must not reference/set resources of the delay slot candidate */
        r2 = mem_buffer[(r4 + 1) & 0xFF] + 1;  /* Independent load+add */
        r5 += r2;
        
    L2:
        /* ----- PATTERN 2: Another conditional jump pattern ----- */
        r1 = r4 * 3;  /* Arithmetic - delay slot candidate */
        
        if (r3 < 4) {
            goto L3;
        }
        
        r5 += r1 | 0x1000;
        goto L4;
        
    L3:
        /* Independent arithmetic after label */
        r2 = (r4 << 2) & 0xFF;  /* Simple shift - won't trap */
        r5 += r2 ^ 0x55;
        
    L4:
        /* ----- PATTERN 3: Volatile access pattern ----- */
        /* Volatile ensures specific resource pattern */
        uint32_t volatile_temp;
        volatile_temp = mem_buffer[r4 & 0xFF];
        r1 = volatile_temp + r4;
        
        if (r3 == 0) {
            goto L5;
        }
        
        r5 += r1 - 10;
        goto L6;
        
    L5:
        /* Independent operation with different register */
        r2 = (r4 * 5) % 256;  /* Simple arithmetic - no traps */
        mem_result = r2;       /* Volatile store creates resource pattern */
        r5 += r2;
        
    L6:
        /* ----- PATTERN 4: Nested condition for multiple slots analysis ----- */
        r1 = (r4 << 1) + 1;
        
        if (r3 != 0) {
            if (r3 < 8) {      /* Nested condition */
                goto L7;
            }
            r5 += r1 * 3;
            goto L8;
        }
        
        r5 += r1;
        goto L8;
        
    L7:
        /* Independent operation using different memory location */
        r2 = mem_buffer[(r4 + 64) & 0xFF] >> 1;  /* Different memory region */
        r5 += r2 & 0x7F;
        
    L8:
        r4++;  /* Loop increment */
    }
    
    /* Use results to prevent optimization */
    printf("Result: %u\n", r5);
    printf("Memory result: %u\n", mem_result);
    
    /* Additional volatile access to ensure all paths matter */
    for (int i = 0; i < 10; i++) {
        mem_buffer[i] += r5;
    }
    
    return (r5 > 1000) ? 0 : 1;
}
