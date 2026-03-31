#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create specific resource patterns */
volatile int g_volatile1 = 1;
volatile int g_volatile2 = 2;
volatile int g_volatile3 = 3;
volatile int g_volatile4 = 4;

int main() NOMIPS16 {
    /* Explicit register variables to control allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int result = 0;
    int i;
    
    /* Initialize register variables */
    r1 = g_volatile1;
    r2 = g_volatile2;
    r3 = g_volatile3;
    r4 = g_volatile4;
    r5 = 0;
    r6 = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        r1 = r1 + 1;  /* Potential delay slot candidate */
        if (r1 > 100) {
            /* Jump to label with independent instruction after */
            goto label1;
        }
        
        /* Some intermediate computation to separate patterns */
        r2 = r2 - 1;
        
        /* Pattern 2: Another conditional jump */
        r3 = r3 & 0xFF;  /* Another delay slot candidate */
        if (r3 != 0) {
            goto label2;
        }
        
        /* Pattern 3: More complex but still simple jump */
        r4 = r4 | 0x55;
        if (r4 < 200) {
            goto label3;
        }
        
        /* Continue loop if no jumps taken */
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r5) than delay slot candidate (r1) */
        r5 = r5 + 2;
        result += r5;
        continue;
        
    label2:
        /* Another independent instruction - different register set */
        r6 = r6 ^ 0xAA;
        result += r6;
        continue;
        
    label3:
        /* Simple arithmetic that doesn't trap or reference shared resources */
        r2 = r2 * 2;
        result += r2;
        /* Continue loop */
    }
    
    /* Pattern 4: Outside loop for variety */
    {
        register int a asm("t6");
        register int b asm("t7");
        register int c asm("t8");
        
        a = g_volatile1;
        b = g_volatile2;
        
        /* Multiple consecutive conditionals to increase slots_to_fill analysis */
        a = a + 5;  /* Delay slot candidate */
        if (a > 50) {
            goto label4;
        }
        
        b = b - 3;  /* Another candidate */
        if (b < 0) {
            goto label5;
        }
        
        c = a & b;  /* Yet another */
        if (c == 0) {
            goto label6;
        }
        
        goto end_pattern;
        
    label4:
        /* Independent load operation */
        c = g_volatile3;
        result += c;
        goto end_pattern;
        
    label5:
        /* Simple arithmetic with volatile */
        c = g_volatile4 + 1;
        result += c;
        goto end_pattern;
        
    label6:
        /* Bit manipulation that can't trap */
        c = (a << 2) | (b >> 1);
        result += c;
        /* fall through */
        
    end_pattern:
        result += a + b + c;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional patterns with memory operations */
    {
        volatile int* ptr = &g_volatile1;
        register int temp asm("t9");
        
        /* Load before conditional */
        temp = *ptr;
        temp = temp + 10;
        
        if (temp > 20) {
            goto label7;
        }
        
        /* Store operation as potential delay slot */
        *ptr = temp;
        if (*ptr != 0) {
            goto label8;
        }
        
        goto final;
        
    label7:
        /* Independent store to different location */
        g_volatile2 = temp;
        result += temp;
        goto final;
        
    label8:
        /* Arithmetic with different register */
        {
            register int x asm("s0");
            x = g_volatile3;
            x = x * x;
            result += x;
        }
        
    final:
        result += g_volatile1;
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
