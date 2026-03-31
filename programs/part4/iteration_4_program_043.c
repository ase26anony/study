/* reorg_coverage.c - Target MIPS delay slot filling for uncovered reorg.cc lines */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create specific resource patterns */
volatile int g_volatile1 = 1;
volatile int g_volatile2 = 2;
volatile int g_volatile3 = 3;
volatile int g_volatile4 = 4;

/* Function to create complex enough control flow for reorg analysis */
NOMIPS16 int test_delay_slots(int iterations) {
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int result = 0;
    int i;
    
    /* Initialize register variables with distinct values */
    r1 = g_volatile1;
    r2 = g_volatile2;
    r3 = g_volatile3;
    r4 = g_volatile4;
    r5 = 0;
    r6 = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        r1 = r1 + g_volatile1;  /* Candidate for delay slot filling */
        if (r1 > 100) {
            /* Jump to label with independent instruction after */
            goto label1;
        }
        /* Fall through path */
        r2 = r2 - g_volatile2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        r3 = r3 & 0xFF;  /* Simple arithmetic, non-trapping */
        result += r3;
        
        /* Pattern 2: Another conditional jump with different registers */
        r4 = r4 * 2;  /* Another delay slot candidate */
        if (r4 < 200) {
            goto label2;
        }
        r5 = r5 | 0x55;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r6 = r6 ^ 0xAA;  /* Different operation, different register */
        result += r6;
        
        /* Pattern 3: Nested conditional jumps */
        r1 = r1 + i;
        if (r1 % 2 == 0) {
            r2 = r2 - i;
            if (r2 > 50) {
                goto label3;
            }
        }
        r3 = r3 + 1;
        continue;
        
    label3:
        /* Independent arithmetic after label */
        r4 = r4 << 1;
        result += r4;
        
        /* Pattern 4: More complex but still simple jump */
        r5 = g_volatile3 + r5;
        if (r5 != r6) {
            goto label4;
        }
        r6 = r6 >> 1;
        continue;
        
    label4:
        /* Simple arithmetic that doesn't reference r5's resources */
        r1 = r1 + 5;
        result += r1;
        
        /* Use results to prevent dead code elimination */
        g_volatile1 = r1;
        g_volatile2 = r2;
        g_volatile3 = r3;
        g_volatile4 = r4;
    }
    
    /* Mix all register results */
    return result + r1 + r2 + r3 + r4 + r5 + r6;
}

/* Helper function to create more jump opportunities */
NOMIPS16 int helper_func(int x, int y) {
    register int a asm("t6");
    register int b asm("t7");
    register int c asm("t8");
    
    a = x;
    b = y;
    
    /* Multiple conditional jumps in sequence */
    if (a > b) {
        a = a - b;
        goto helper_label1;
    }
    b = b - a;
    
helper_label1:
    /* Independent instruction after label */
    c = a + b;
    
    if (c > 100) {
        c = c >> 1;
        goto helper_label2;
    }
    c = c << 1;
    
helper_label2:
    /* Another independent instruction */
    a = a ^ b;
    
    return a + b + c;
}

/* Main function with loop to ensure reorg pass is invoked */
NOMIPS16 int main() {
    int total = 0;
    int i;
    
    printf("Testing MIPS delay slot filling patterns...\n");
    
    /* Multiple iterations to give reorg pass enough work */
    for (i = 0; i < 1000; i++) {
        total += test_delay_slots(10);
        total += helper_func(i, i * 2);
        
        /* Volatile memory operations to create resource conflicts */
        g_volatile1 = g_volatile1 + 1;
        g_volatile2 = g_volatile2 - 1;
        g_volatile3 = g_volatile3 * 2;
        g_volatile4 = g_volatile4 / 2;
    }
    
    printf("Result: %d\n", total);
    printf("Volatile values: %d %d %d %d\n", 
           g_volatile1, g_volatile2, g_volatile3, g_volatile4);
    
    return 0;
}
