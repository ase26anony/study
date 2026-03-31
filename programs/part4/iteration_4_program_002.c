#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 0x1234;
    volatile int mem2 = 0x5678;
    volatile int mem3 = 0x9ABC;
    volatile int mem4 = 0xDEF0;
    
    /* Explicit register variables to control resource allocation */
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
    
    /* Initialize registers with volatile memory loads */
    r1 = mem1;
    r2 = mem2;
    r3 = mem3;
    r4 = mem4;
    r5 = 0;
    r6 = 0;
    r7 = 0;
    r8 = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 100; i++) {
        /* BLOCK 1: Candidate for delay slot filling */
        /* r5 is modified here - potential delay slot instruction */
        r5 = r1 + r2;
        
        /* Simple conditional jump to label1 */
        /* This should compile to simplejump_p */
        if (r1 > r2) {
            goto label1;
        }
        
        /* Independent instruction using different register set */
        r6 = r3 & r4;
        continue;
        
    label1:
        /* This is next_trial - instruction after the jump label */
        /* Uses different registers (r7, r8) than the delay slot candidate (r5) */
        /* Simple arithmetic that doesn't trap */
        r7 = r8 + 1;
        
        /* Accumulate result to prevent dead code elimination */
        result += r5 + r6 + r7;
        
        
        /* BLOCK 2: Another pattern with different resource usage */
        /* Delay slot candidate using r6 */
        r6 = r3 - r4;
        
        /* Another simple conditional jump */
        if (r3 != r4) {
            goto label2;
        }
        
        r5 = r1 | r2;
        continue;
        
    label2:
        /* next_trial with different resource set */
        r8 = r7 ^ 0xFF;
        result += r6 + r8;
        
        
        /* BLOCK 3: Pattern with memory operations */
        /* Volatile load as delay slot candidate */
        int temp = mem1;
        
        if (temp > 1000) {
            goto label3;
        }
        
        r1 = r2 * 2;
        continue;
        
    label3:
        /* Independent arithmetic after label */
        r2 = r3 + r4;
        result += temp + r2;
        
        
        /* BLOCK 4: Pattern with increment and test */
        r3 = r3 + 1;
        
        if (r3 < 50) {
            goto label4;
        }
        
        r4 = r4 - 1;
        continue;
        
    label4:
        r1 = r2 & 0x0F;
        result += r3 + r1;
        
        
        /* BLOCK 5: Nested condition pattern */
        r4 = r4 + 2;
        
        if (r4 > 25) {
            if (r1 < 10) {
                goto label5;
            }
        }
        
        r2 = r3 >> 1;
        continue;
        
    label5:
        r5 = r6 << 1;
        result += r4 + r5;
        
        /* Modify registers to change conditions */
        r1 = (r1 + 1) & 0x7F;
        r2 = (r2 + 2) & 0xFF;
    }
    
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d, t6=%d, t7=%d\n",
           r1, r2, r3, r4, r5, r6, r7, r8);
    
    return 0;
}
