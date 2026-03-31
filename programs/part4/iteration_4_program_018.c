/* reorg_delay_slot_test.c
 * Test program to trigger uncovered delay slot filling logic in GCC reorg pass
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[16] = {0};

/* Function to create complex control flow with delay slot opportunities */
NOMIPS16 int test_delay_slot_filling(void) {
    /* Register variables to control register allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int result = 0;
    int i, j;
    
    /* Initialize registers with distinct values */
    r1 = 1;
    r2 = 2;
    r3 = 3;
    r4 = 4;
    r5 = 5;
    r6 = 6;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 100; i++) {
        /* Force compiler to keep these values live */
        g_volatile_counter = i;
        
        /* Pattern 1: Simple conditional jump with independent instruction after label */
        if (r1 > 0) {
            /* This arithmetic could be a delay slot candidate */
            r1 = r1 + r2;  /* Uses t0 and t1 */
            goto label1;
        }
        
        /* Some code to avoid fall-through optimization */
        r6 = r6 ^ 0x55;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different registers (t2, t3) than the delay slot candidate */
        r3 = r3 & r4;  /* Should not conflict with r1+r2 */
        result += r3;
        
        /* Pattern 2: Another conditional jump with different registers */
        if (r2 < 100) {
            /* Different register set for delay slot candidate */
            r5 = r5 - r4;  /* Uses t4 and t3 */
            goto label2;
        }
        
        r6 = r6 ^ 0xAA;
        
    label2:
        /* Another independent instruction after label */
        /* Uses t0 and t5 - careful not to overlap with r5-r4 */
        r1 = r1 | r6;  /* Different registers than the jump's delay slot */
        result += r1;
        
        /* Pattern 3: Conditional with volatile memory access */
        if (g_volatile_array[i & 0xF] != 0) {
            /* Volatile load as delay slot candidate */
            int temp = g_volatile_array[(i + 1) & 0xF];
            goto label3;
        }
        
        r2 = r2 * 2;
        
    label3:
        /* Simple arithmetic after label - no memory ops to avoid traps */
        r4 = r4 ^ 0xFF;  /* Simple, non-trapping operation */
        result += r4;
        
        /* Pattern 4: Nested conditionals to create more complex CFG */
        for (j = 0; j < 4; j++) {
            if (r3 > r4) {
                /* Arithmetic with different register set */
                r2 = r2 + r5;  /* t1 + t4 */
                goto label4;
            }
            
            r6 = r6 - 1;
            
        label4:
            /* Independent instruction - uses t0 which isn't set by r2+r5 */
            r1 = r1 << 1;  /* Shift operation */
            result += r1;
            
            /* Another immediate jump opportunity */
            if (r6 != 0) {
                r3 = r3 >> 1;  /* Different operation */
                goto label5;
            }
            
            r4 = r4 + 1;
            
        label5:
            /* Final independent instruction in the chain */
            r5 = r5 & 0x7F;  /* Mask operation */
            result += r5;
        }
        
        /* Modify values to change branch behavior */
        r1 = (r1 * 3) & 0xFF;
        r2 = (r2 + i) & 0xFF;
        r3 = (r3 ^ i) & 0xFF;
        r4 = (r4 - i) & 0xFF;
        r5 = (r5 | i) & 0xFF;
        r6 = (r6 + 1) & 0xFF;
    }
    
    return result;
}

/* Main function to run the test */
int main(void) {
    int result;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Initialize volatile array with pattern */
    for (int i = 0; i < 16; i++) {
        g_volatile_array[i] = i * i;
    }
    
    /* Run the test multiple times to ensure reorg pass is invoked */
    for (int run = 0; run < 3; run++) {
        result = test_delay_slot_filling();
        printf("Run %d: Result = %d\n", run + 1, result);
        
        /* Modify volatile data to change execution path */
        g_volatile_counter += 1000;
        for (int i = 0; i < 16; i++) {
            g_volatile_array[i] += run;
        }
    }
    
    printf("Test completed.\n");
    return 0;
}
