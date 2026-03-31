/* reorg_delay_slot_test.c
 * Designed to trigger GCC's delay slot filling logic for MIPS targets
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-reorg -o test test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which may not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Function to create complex enough control flow for reorg analysis */
NOMIPS16 int test_delay_slots(int iterations) {
    /* Use explicit register variables to control resource allocation */
    register int r0 asm("t0") = 0;  /* Candidate delay slot reg */
    register int r1 asm("t1") = 1;  /* Independent reg for next_trial */
    register int r2 asm("t2") = 2;  /* Another independent reg */
    register int r3 asm("t3") = 3;  /* Yet another independent reg */
    
    int result = 0;
    int i;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < iterations; i++) {
        /* Create a volatile memory access that could be a delay slot candidate */
        int temp = g_volatile_array[i & 0xF];
        
        /* First if-goto construct with simple condition */
        if (r0 > 0) {
            /* This arithmetic could be moved into delay slot */
            r0 = r0 + temp + 1;
            /* Jump to label L1 - creates simplejump_p */
            goto L1;
        }
        /* Fall-through path */
        r0 = r0 - temp;
        continue;
        
    L1:
        /* This is next_trial - must be independent of r0 operations */
        /* Uses different register (r1) to avoid resource conflicts */
        r1 = r1 * 2;
        result += r1;
        
        /* Second if-goto construct with different condition */
        if (r2 < 100) {
            /* Another potential delay slot candidate */
            r2 = r2 + g_volatile_counter;
            /* Jump to label L2 */
            goto L2;
        }
        r2 = r2 - 1;
        continue;
        
    L2:
        /* Another independent next_trial using r3 */
        r3 = r3 & 0xFF;  /* Simple arithmetic, cannot trap */
        result += r3;
        
        /* Third if-goto with volatile condition */
        if (g_volatile_counter != 0) {
            /* Volatile access as delay slot candidate */
            g_volatile_counter = g_volatile_counter + 1;
            /* Jump to label L3 */
            goto L3;
        }
        g_volatile_counter = g_volatile_counter - 1;
        continue;
        
    L3:
        /* Independent arithmetic for next_trial */
        r1 = r1 ^ r3;  /* XOR is simple and non-trapping */
        result += r1;
        
        /* Update loop variables using different registers */
        r0 = r0 + i;
        r2 = r2 + i;
    }
    
    /* Mix all register results to prevent dead code elimination */
    result = result + r0 + r1 + r2 + r3;
    return result;
}

/* Helper function to create more complex control flow */
NOMIPS16 int nested_condition_test(int x, int y) {
    register int a asm("t4") = x;
    register int b asm("t5") = y;
    register int c asm("t6") = 0;
    
    /* Multiple nested conditions to create jump chains */
    if (a > 0) {
        /* Potential delay slot: increment using volatile */
        g_volatile_array[0] = g_volatile_array[0] + 1;
        goto NESTED_L1;
    }
    a = a * 2;
    
NESTED_L1:
    /* Independent operation after label */
    b = b << 1;
    
    if (b < a) {
        /* Another potential delay slot */
        c = a - b;
        goto NESTED_L2;
    }
    c = b - a;
    
NESTED_L2:
    /* Another independent operation */
    a = a | b;  /* OR operation is simple and safe */
    
    /* One more if-goto to increase opportunities */
    if (c != 0) {
        g_volatile_counter = g_volatile_counter + c;
        goto NESTED_L3;
    }
    g_volatile_counter = g_volatile_counter - 1;
    
NESTED_L3:
    /* Final independent operation */
    b = b + 1;
    
    return a + b + c;
}

int main() {
    int i, total = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Initialize volatile array with pattern */
    for (i = 0; i < 16; i++) {
        g_volatile_array[i] = i * 3;
    }
    
    /* Run multiple iterations to ensure reorg pass is invoked */
    for (i = 0; i < 10; i++) {
        int res1 = test_delay_slots(100);
        int res2 = nested_condition_test(i, i * 2);
        
        total += res1 + res2;
        
        /* Modify volatile to change control flow */
        g_volatile_counter = (g_volatile_counter + 1) & 0x7;
    }
    
    printf("Accumulated result: %d\n", total);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
