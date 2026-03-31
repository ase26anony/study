/* Target: MIPS with delay slot filling */
/* Compile with: -O2 -march=mips1 -fdump-rtl-reorg */
/* For execution: -O2 -march=mips1 -fsanitize=address */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which may not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    volatile int v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 100;
    register int r2 asm("t1") = 200;
    register int r3 asm("t2") = 300;
    register int r4 asm("t3") = 400;
    register int r5 asm("t4") = 500;
    register int r6 asm("t5") = 600;
    
    int result = 0;
    int i;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This creates a delay slot candidate (the add) */
        r1 = r1 + v1;  /* Potential delay slot instruction */
        if (r1 > 50) {
            /* Jump target label with independent instruction */
            target1:
            /* Independent arithmetic - uses different registers */
            r2 = r2 & 0xFF;  /* This is next_trial candidate */
            result += r2;
        }
        
        /* Pattern 2: Another conditional jump pattern */
        r3 = r3 - v2;  /* Delay slot candidate */
        if (r3 < 1000) {
            target2:
            /* Independent load operation */
            r4 = v3 + v4;  /* next_trial candidate */
            result += r4;
        }
        
        /* Pattern 3: goto with label immediately after */
        r5 = r5 | 0x0F;  /* Delay slot candidate */
        if (v5 > 0) {
            goto target3;
        }
        /* Some code to avoid fall-through optimization */
        r6 = r6 ^ 0x55;
        
        target3:
        /* Simple arithmetic that doesn't reference r5's resources */
        r1 = r1 * 2;  /* next_trial candidate */
        result += r1;
        
        /* Pattern 4: Nested conditionals to create complex flow */
        r2 = r2 + i;  /* Delay slot candidate */
        if (v6 != 0) {
            if (r2 % 2 == 0) {
                target4:
                /* Independent store/volatile operation */
                v7 = r3 + 1;  /* next_trial candidate */
                result += v7;
            }
        }
        
        /* Pattern 5: Multiple consecutive jumps */
        r4 = r4 >> 1;  /* Delay slot candidate 1 */
        if (v8 > i) {
            target5:
            r5 = r5 << 1;  /* next_trial candidate 1 */
            result += r5;
        }
        
        r6 = r6 + 1;  /* Delay slot candidate 2 */
        if (result < 10000) {
            target6:
            r1 = r1 - 1;  /* next_trial candidate 2 */
            result += r1;
        }
        
        /* Pattern 6: Switch-like pattern with goto labels */
        r3 = r3 & ~0x01;  /* Delay slot candidate */
        switch (i % 3) {
            case 0:
                goto case0;
            case 1:
                goto case1;
            default:
                goto case2;
        }
        
        case0:
        r2 = v1 * 2;  /* next_trial candidate */
        result += r2;
        continue;
        
        case1:
        r4 = v2 * 3;
        result += r4;
        continue;
        
        case2:
        r6 = v3 * 4;
        result += r6;
        continue;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d\n", r1, r2, r3, r4, r5, r6);
    printf("Volatiles: %d %d %d %d\n", v1, v2, v3, v4);
    
    return 0;
}
