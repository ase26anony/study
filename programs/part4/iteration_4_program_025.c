/* Target: MIPS with delay slots - designed to trigger reorg.cc lines 2135-2149 */
#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create specific resource patterns */
volatile int g_counter = 0;
volatile int g_array[256] = {0};
volatile int g_result = 0;

NOMIPS16 int main(void) {
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 1;  /* Delay slot candidate register */
    register int r2 asm("t1") = 2;  /* Independent register for next_trial */
    register int r3 asm("t2") = 3;  /* Another independent register */
    register int r4 asm("t3") = 4;  /* Yet another independent register */
    
    int i, j;
    
    /* Initialize array with some values */
    for (i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    
    /* Main loop with multiple conditional jumps - creates many delay slot opportunities */
    for (i = 0; i < 1000; i++) {
        /* First if-goto block with simple condition */
        if (r1 > 0) {
            /* This arithmetic could be a delay slot candidate */
            r1 = r1 + g_array[i & 255];
            goto label1;
        }
        /* Fall-through path */
        r3 = r3 - 1;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r2) than the delay slot candidate (r1) */
        r2 = r2 & 0xFF;
        g_result += r2;
        
        
        /* Second if-goto block with different registers */
        if (r3 < 100) {
            /* Another delay slot candidate opportunity */
            r3 = r3 * 2;
            goto label2;
        }
        /* Fall-through path */
        r4 = r4 + 1;
        continue;
        
    label2:
        /* Another independent instruction after label */
        /* Uses volatile memory to create specific resource pattern */
        g_counter = r4;
        r4 = g_counter + 1;
        
        
        /* Third if-goto block with volatile access */
        if (g_array[(i + 1) & 255] > 500) {
            /* Delay slot candidate with memory access */
            volatile int temp = g_array[i & 255];
            (void)temp;  /* Use the value to prevent elimination */
            goto label3;
        }
        /* Fall-through path */
        r1 = r1 | 0x1;
        continue;
        
    label3:
        /* Simple arithmetic after label - very likely eligible */
        r2 = r2 + r3;
        
        
        /* Fourth if-goto block - nested condition */
        if ((r1 & 1) == 0) {
            if (r2 > r3) {
                /* Nested jumps create interesting control flow */
                r4 = r4 ^ r1;
                goto label4;
            }
        }
        /* Fall-through path */
        r3 = r3 >> 1;
        continue;
        
    label4:
        /* Independent arithmetic with no resource conflicts */
        r1 = r1 * 3;
        
        
        /* Fifth if-goto block - loop condition style */
        if (i < 500) {
            /* Simple arithmetic that doesn't trap */
            r2 = r2 - 5;
            goto label5;
        }
        /* Fall-through path */
        r4 = r4 + g_array[0];
        continue;
        
    label5:
        /* Final independent operation */
        r3 = r3 & ~0x3;
        
        /* Accumulate results to prevent dead code elimination */
        g_result += r1 + r2 + r3 + r4;
    }
    
    /* Print results to prevent optimization */
    printf("Result: %d\n", g_result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d\n", r1, r2, r3, r4);
    printf("Counter: %d\n", g_counter);
    
    return 0;
}
