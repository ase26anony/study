/* reorg_delay_slot_test.c
 * Target: MIPS with delay slots (-march=mips1)
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-reorg -o test test.c
 * Or for QEMU: mips-linux-gnu-gcc -O2 -march=mips1 -static -o test test.c && qemu-mips ./test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile memory locations to create specific resource patterns */
volatile int mem1 = 100;
volatile int mem2 = 200;
volatile int mem3 = 300;
volatile int mem4 = 400;

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

NOMIPS16 int main(void) {
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");  /* $t0 - temp register 0 */
    register int r2 asm("t1");  /* $t1 - temp register 1 */
    register int r3 asm("t2");  /* $t2 - temp register 2 */
    register int r4 asm("t3");  /* $t3 - temp register 3 */
    register int r5 asm("t4");  /* $t4 - temp register 4 */
    register int r6 asm("t5");  /* $t5 - temp register 5 */
    register int cond asm("t6"); /* $t6 - condition register */
    
    int i, result = 0;
    
    /* Initialize registers with values that will create interesting conditions */
    r1 = mem1;  /* Load volatile value into t0 */
    r2 = mem2;  /* Load volatile value into t1 */
    r3 = mem3;  /* Load volatile value into t2 */
    r4 = mem4;  /* Load volatile value into t3 */
    r5 = 0;     /* Initialize t4 */
    r6 = 0;     /* Initialize t5 */
    
    /* Loop to increase chance of reorg pass analysis */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple arithmetic that could be a delay slot candidate */
        /* This creates an instruction that might be scheduled into delay slot */
        r1 = r1 + i;  /* t0 = t0 + i - uses t0 and immediate */
        
        /* Conditional jump to label L1 - should compile to simplejump_p */
        /* The condition uses different registers than the arithmetic above */
        cond = r2 - r3;  /* t6 = t1 - t2 */
        if (cond > 0) {
            /* This goto creates a jump to label with simple condition */
            goto L1;
        }
        
        /* Fall-through path */
        r4 = r4 & 0xFF;  /* Independent operation in fall-through */
        continue;
        
    L1:
        /* Instruction at jump target (next_trial candidate) */
        /* Must be: NONJUMP_INSN_P, not SEQUENCE, not JUMP_P, 
           not reference/set conflicting resources, not throw */
        r5 = r5 ^ 0x5555;  /* t4 = t4 ^ 0x5555 - uses different register set */
                           /* Simple integer arithmetic, no trapping */
        
        /* Another pattern with different registers */
        r3 = r3 - 1;  /* t2 = t2 - 1 - delay slot candidate */
        cond = r1 - r4;  /* t6 = t0 - t3 */
        if (cond != 0) {
            goto L2;
        }
        r6 = r6 | 0xAAAA;
        continue;
        
    L2:
        /* Another independent instruction at jump target */
        r2 = r2 + r5;  /* t1 = t1 + t4 - uses t1 and t4, different from above */
        
        /* Pattern 3: Memory operation as delay slot candidate */
        /* Volatile load creates specific resource pattern */
        r6 = mem1;  /* t5 = load from mem1 - volatile ensures non-optimizable */
        cond = r3 - 10;  /* t6 = t2 - 10 */
        if (cond < 0) {
            goto L3;
        }
        r1 = r1 >> 2;
        continue;
        
    L3:
        /* Arithmetic at target using completely different register */
        r4 = r4 * 2;  /* t3 = t3 * 2 - independent operation */
        
        /* Pattern 4: Multiple consecutive conditionals */
        r5 = r5 + i;  /* Potential delay slot instruction */
        cond = r6 & 1;  /* Check LSB */
        if (cond == 0) {
            goto L4;
        }
        
        r2 = r2 - i;
        goto L5;  /* Skip L4 */
        
    L4:
        /* Target with simple arithmetic */
        r3 = r3 | 0xF0F0;  /* t2 = t2 | 0xF0F0 */
        
    L5:
        /* Continue the loop */
        result += r1 + r2 + r3 + r4 + r5 + r6;
    }
    
    /* Accumulate results to prevent optimization */
    global_acc = result;
    
    /* Use results to ensure they're not dead */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r1, r2, r3, r4, r5, r6);
    
    return 0;
}
