#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
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
    
    /* Initialize registers with values */
    r1 = mem1;
    r2 = mem2;
    r3 = mem3;
    r4 = mem4;
    r5 = 0;
    r6 = 0;
    r7 = 0;
    r8 = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple arithmetic that could be a delay slot candidate */
        r1 = r1 + 1;
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r1 > 1000) {
            goto L1;
        }
        
        /* Independent arithmetic after jump target (potential next_trial) */
        /* Uses different registers (r2, r3) than the delay slot candidate (r1) */
        r2 = r2 & 0xFF;
        r3 = r3 | 0x55;
        
        /* Continue to avoid label being optimized away */
        result += r2 + r3;
        continue;
        
    L1:
        /* This is the instruction after the jump label (next_trial candidate) */
        /* Uses register r4 which doesn't conflict with r1 */
        r4 = r4 - 1;
        result += r4;
        
        /* Pattern 2: Another independent operation */
        r5 = r5 ^ 0xAA;
        
        /* Another conditional jump to label L2 */
        if (r4 < -500) {
            goto L2;
        }
        
        /* More independent operations */
        r6 = r6 << 2;
        result += r6;
        continue;
        
    L2:
        /* Another next_trial candidate after L2 */
        /* Uses r7 which doesn't conflict with previous operations */
        r7 = r7 + 3;
        result += r7;
        
        /* Pattern 3: Memory operation as delay slot candidate */
        /* Volatile access ensures specific resource pattern */
        r8 = mem1;
        
        /* Conditional jump to label L3 */
        if (r8 != 0x1234) {
            goto L3;
        }
        
        /* Independent operation */
        r1 = r1 * 2;
        result += r1;
        continue;
        
    L3:
        /* next_trial after L3 - simple arithmetic */
        r2 = r2 / 2;
        result += r2;
        
        /* Pattern 4: Complex enough to require multiple delay slot attempts */
        r3 = r3 % 100;
        
        /* Nested conditional jumps to create multiple slot filling opportunities */
        if (r3 > 50) {
            if (r2 < 100) {
                goto L4;
            }
        }
        
        r4 = r4 + 5;
        result += r4;
        continue;
        
    L4:
        /* next_trial after L4 */
        r5 = r5 | 0xF0;
        result += r5;
        
        /* Reset some values to continue loop */
        if (i % 10 == 0) {
            r1 = mem1;
            r2 = mem2;
        }
    }
    
    /* Use all results to prevent dead code elimination */
    result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    
    /* Add memory results */
    result += mem1 + mem2 + mem3 + mem4;
    
    printf("Result: %d\n", result);
    return 0;
}
