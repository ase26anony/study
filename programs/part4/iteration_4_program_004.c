/* reorg_delay_slot_test.c
 * Designed to trigger GCC's delay slot filling logic in reorg.cc lines 2135-2149
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2 -o test test.c
 * Run with: qemu-mips test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Function to ensure operations aren't optimized away */
static NOMIPS16 void use_value(int value) {
    g_volatile_counter += value;
}

int main(void) {
    /* Explicit register variables to control resource allocation */
    register int r0 asm("t0") = 1;  /* Candidate delay slot register */
    register int r1 asm("t1") = 2;  /* Independent register for next_trial */
    register int r2 asm("t2") = 3;  /* Another independent register */
    register int r3 asm("t3") = 4;  /* Yet another independent register */
    
    /* Volatile locals to force memory operations */
    volatile int v1 = 10;
    volatile int v2 = 20;
    volatile int v3 = 30;
    
    int result = 0;
    int i;
    
    /* Loop to increase chances of reorg pass analysis */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple arithmetic that could be delay slot candidate */
        r0 = r0 + i;  /* This might be scheduled into delay slot */
        
        /* Conditional jump to label - must compile to simplejump_p */
        if (v1 > 5) {
            /* Target label must be immediately after jump */
            goto label1;
        }
        
        /* Fall-through path */
        r2 = r2 * 2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than delay slot candidate (r0) */
        r1 = r1 + v2;  /* Should not reference/set resources used by r0 */
        
        /* Use result to prevent elimination */
        result += r0 + r1;
        
        
        /* Pattern 2: Another independent pattern */
        r3 = r3 - i;
        
        if (v2 < 25) {
            goto label2;
        }
        
        r0 = r0 / 2;
        continue;
        
    label2:
        /* Another independent instruction after label */
        /* Uses volatile memory to create specific resource pattern */
        g_volatile_array[i & 0xF] = r2;
        r2 = g_volatile_array[(i + 1) & 0xF] + 1;
        
        result += r2 + r3;
        
        
        /* Pattern 3: More complex but still simple jump */
        v3 = v3 + 1;
        
        if (i % 3 == 0) {
            goto label3;
        }
        
        r1 = r1 ^ 0xFF;
        continue;
        
    label3:
        /* Arithmetic with immediate - won't trap */
        r3 = r3 & 0x7FFFFFFF;  /* Non-trapping operation */
        
        result += r3;
        
        
        /* Pattern 4: Nested control flow to create multiple slot opportunities */
        if (result > 1000) {
            /* Reset to avoid overflow */
            result = result / 2;
        }
        
        if (i % 5 == 0) {
            goto label4;
        }
        
        r0 = r0 | 0x01;
        continue;
        
    label4:
        /* Simple load operation - independent of previous operations */
        int temp = g_volatile_counter;
        r1 = temp + 1;
        
        result += r1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Register values: t0=%d, t1=%d, t2=%d, t3=%d\n", r0, r1, r2, r3);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
