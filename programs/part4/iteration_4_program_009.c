/* reorg_delay_slot.c
 * 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the reorg pass, specifically targeting the uncovered lines 2135-2149
 * in reorg.cc.
 *
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o reorg_test reorg_delay_slot.c
 * Run with QEMU: qemu-mips ./reorg_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Function to create register pressure and specific resource usage */
NOMIPS16 int test_delay_slot_filling(int iterations) {
    /* Explicit register variables to control register allocation */
    register int r1 asm("t0") = 0;  /* Candidate delay slot reg */
    register int r2 asm("t1") = 0;  /* Independent reg for next_trial */
    register int r3 asm("t2") = 0;  /* Another independent reg */
    register int r4 asm("t3") = 0;  /* Condition testing reg */
    
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple arithmetic that could be a delay slot candidate */
        r1 = g_volatile_array[i & 0xF] + 1;
        
        /* Conditional jump with simple condition - should become simplejump_p */
        r4 = g_volatile_counter++;
        if (r4 > 100) {
            /* Jump to label L1 - this creates jump_to_label_p */
            goto L1;
        }
        
        /* Fall-through path */
        r3 = r2 * 2;
        result += r3;
        continue;
        
    L1:
        /* Instruction after label: independent arithmetic operation
         * This is the potential next_trial for delay slot filling.
         * Uses different registers (r2) than the delay slot candidate (r1)
         * to avoid resource conflicts.
         */
        r2 = r3 + 5;  /* Simple integer arithmetic, non-trapping */
        
        /* Avoid jumps, sequences, or throwing instructions here */
        result += r1 + r2;
        
        /* Another pattern to increase opportunities */
        r1 = g_volatile_array[(i + 1) & 0xF] - 1;
        
        /* Second conditional jump in tight sequence */
        if (r2 < 50) {
            goto L2;
        }
        
        r3 = r1 / 2;
        result += r3;
        continue;
        
    L2:
        /* Another independent instruction after label */
        r3 = r4 & 0xFF;  /* Bitwise operation, non-trapping */
        result += r3;
    }
    
    /* Third pattern with different register usage */
    for (i = 0; i < iterations / 2; i++) {
        register int r5 asm("t4") = i * 3;
        register int r6 asm("t5") = g_volatile_counter;
        
        /* Delay slot candidate using volatile memory */
        g_volatile_array[i & 0xF] = r5 + r6;
        
        /* Conditional jump */
        if (r6 != 0) {
            goto L3;
        }
        
        r1 = r5 - r6;
        result += r1;
        continue;
        
    L3:
        /* Independent instruction using completely different register */
        register int r7 asm("t6") = r6 | 0x55;
        r2 = r7 * 3;
        result += r2;
        
        /* Nested conditional to create more jump opportunities */
        if (r2 > 1000) {
            goto L4;
        }
        
        r3 = r1 + r2;
        result += r3;
        continue;
        
    L4:
        /* Simple arithmetic after label */
        r4 = r7 ^ 0xAA;
        result += r4;
    }
    
    return result;
}

/* Additional function to create more complex control flow */
NOMIPS16 int nested_conditionals(int limit) {
    register int a asm("s0") = 0;
    register int b asm("s1") = 0;
    register int c asm("s2") = 0;
    int total = 0;
    
    while (a < limit) {
        /* Volatile access creates specific resource pattern */
        b = g_volatile_array[a & 0xF];
        
        /* Multiple conditional jumps in sequence */
        if (b > 50) {
            goto NESTED_L1;
        }
        
        c = a * 2;
        total += c;
        a++;
        continue;
        
    NESTED_L1:
        /* Independent instruction after label */
        c = b - 10;
        
        /* Another conditional */
        if (c < 0) {
            goto NESTED_L2;
        }
        
        total += c;
        a++;
        continue;
        
    NESTED_L2:
        /* Another independent instruction */
        register int d asm("s3") = c * 3;
        total += d;
        a++;
    }
    
    return total;
}

int main(void) {
    int i;
    
    /* Initialize volatile array */
    for (i = 0; i < 16; i++) {
        g_volatile_array[i] = i * 10;
    }
    
    /* Run tests with enough iterations to trigger reorg pass */
    int result1 = test_delay_slot_filling(1000);
    int result2 = nested_conditionals(500);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
