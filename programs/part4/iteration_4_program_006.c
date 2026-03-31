/* reorg_delay_slot_test.c
 * Test program to trigger uncovered delay slot filling logic in GCC's reorg pass
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-reorg -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create specific resource patterns */
volatile int g_volatile1 = 1;
volatile int g_volatile2 = 2;
volatile int g_volatile3 = 3;
volatile int g_volatile4 = 4;

/* Function with multiple conditional jumps to create delay slot opportunities */
NOMIPS16 static int test_delay_slots(int iterations) {
    /* Use explicit register variables to control resource allocation */
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
    
    /* Initialize register variables with distinct values */
    r1 = g_volatile1;
    r2 = g_volatile2;
    r3 = g_volatile3;
    r4 = g_volatile4;
    r5 = 5;
    r6 = 6;
    r7 = 7;
    r8 = 8;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This arithmetic could be moved into delay slot */
        r1 = r1 + r2;  /* Candidate for delay slot filling */
        
        if (r1 > 0) {
            /* Jump to label with independent instruction after */
            goto label1;
        }
        
        /* Fall-through path */
        r3 = r3 - r4;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers than the delay slot candidate */
        r5 = r5 & r6;  /* Should not conflict with r1+r2 */
        result += r5;
        
        /* Pattern 2: Another conditional jump with different registers */
        r2 = r2 * r3;  /* Another delay slot candidate */
        
        if (r2 != 0) {
            goto label2;
        }
        
        r4 = r4 | r1;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r7 = r7 ^ r8;  /* Different register set */
        result += r7;
        
        /* Pattern 3: Conditional with volatile memory access */
        g_volatile1 = r1 + r5;  /* Volatile store as delay slot candidate */
        
        if (r3 < r4) {
            goto label3;
        }
        
        g_volatile2 = r2 + r6;
        continue;
        
    label3:
        /* Load from volatile after label */
        r8 = g_volatile3;  /* Independent volatile load */
        result += r8;
        
        /* Pattern 4: More complex condition but still simple jump */
        r4 = r4 << 2;  /* Shift operation as candidate */
        
        if ((r1 & 0xFF) == 0) {
            goto label4;
        }
        
        r6 = r6 >> 1;
        continue;
        
    label4:
        /* Simple arithmetic after label */
        r2 = r2 + 1;  /* Independent increment */
        result += r2;
        
        /* Pattern 5: Nested resource pattern */
        r3 = r3 - g_volatile4;  /* Volatile load in candidate */
        
        if (r5 > r7) {
            goto label5;
        }
        
        r1 = r1 * 3;
        continue;
        
    label5:
        /* Store to volatile after label */
        g_volatile2 = r8;  /* Independent volatile store */
        result += g_volatile2;
        
        /* Mix up register values to create varying conditions */
        r1 = r1 ^ result;
        r2 = r2 + i;
        r3 = r3 | 0x55;
        r4 = r4 & 0xAA;
    }
    
    /* Ensure all results are used to prevent optimization */
    result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return result;
}

/* Helper function to create more jump opportunities */
NOMIPS16 static int helper_func(int x, int y) {
    register int a asm("s0");
    register int b asm("s1");
    register int c asm("s2");
    
    a = x;
    b = y;
    c = 0;
    
    /* Multiple conditional jumps in sequence */
    if (a > b) {
        a = a - b;
        goto helper_label1;
    }
    
    b = b - a;
    return a + b;
    
helper_label1:
    /* Independent instruction after label */
    c = a * 2;
    
    if (b > 10) {
        b = b >> 1;
        goto helper_label2;
    }
    
    a = a << 1;
    return c;
    
helper_label2:
    /* Another independent instruction */
    c = c + 5;
    return c;
}

NOMIPS16 int main(void) {
    int i, total_result = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Call multiple times to ensure reorg pass analyzes the code */
    for (i = 0; i < 100; i++) {
        int result = test_delay_slots(50);
        total_result += result;
        
        /* Also call helper to create more jump patterns */
        total_result += helper_func(i, i * 2);
    }
    
    printf("Total result: %d\n", total_result);
    printf("If you see this, the program compiled and ran successfully.\n");
    
    /* Use the result to prevent dead code elimination */
    if (total_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
