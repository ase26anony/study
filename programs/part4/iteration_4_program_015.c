/* reorg_delay_slot_test.c
 * Target: MIPS with delay slot filling enabled
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 * Run with: qemu-mips ./test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create specific resource patterns */
volatile int g_volatile1 = 123;
volatile int g_volatile2 = 456;
volatile int g_volatile3 = 789;
volatile int g_volatile4 = 1011;

/* Function to create complex control flow with eligible delay slot candidates */
NOMIPS16 static int test_delay_slot_pattern(int iterations) {
    /* Use explicit register variables to control resource allocation */
    register int r0 asm("t0") = 0;  /* Candidate delay slot register */
    register int r1 asm("t1") = 0;  /* Independent register for next_trial */
    register int r2 asm("t2") = 0;  /* Another independent register */
    register int r3 asm("t3") = 0;  /* Yet another independent register */
    
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple arithmetic that could fill delay slot */
        r0 = g_volatile1 + i;  /* This could be moved into delay slot */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r0 > 100) {
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r2 = g_volatile2 * 2;
        result += r2;
        continue;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r1) than the delay slot candidate (r0) */
        r1 = g_volatile3 & 0xFF;  /* Simple, non-trapping operation */
        result += r1;
        
        /* Pattern 2: Another candidate with different registers */
        r3 = g_volatile4 - i;  /* Another delay slot candidate */
        
        if (r3 < 500) {
            goto L2;
        }
        
        r2 = g_volatile1 | 0x55;
        result += r2;
        continue;
        
    L2:
        /* Another independent instruction after label */
        r1 = g_volatile2 ^ 0xAA;  /* Different operation, same register set */
        result += r1;
        
        /* Pattern 3: Nested conditionals to create more jump opportunities */
        r0 = g_volatile3 + g_volatile4;
        
        if (r0 != 0) {
            if ((i & 1) == 0) {
                goto L3;
            }
        }
        
        r2 = g_volatile1 << 2;
        result += r2;
        continue;
        
    L3:
        /* Independent arithmetic after label */
        r1 = g_volatile4 >> 1;
        result += r1;
        
        /* Pattern 4: Memory access pattern */
        volatile int local_volatile = i * 2;
        r0 = local_volatile + g_volatile1;
        
        if (r0 > local_volatile) {
            goto L4;
        }
        
        r2 = g_volatile2 + g_volatile3;
        result += r2;
        continue;
        
    L4:
        /* Simple arithmetic that doesn't reference r0's resources */
        r1 = g_volatile4 * 3;
        result += r1;
        
        /* Pattern 5: Loop with multiple exit points */
        r3 = i * i;
        
        if (r3 > 1000) {
            goto L5;
        }
        
        if (r3 < 100) {
            goto L6;
        }
        
        r2 = g_volatile1 % 17;
        result += r2;
        continue;
        
    L5:
        r1 = g_volatile2 + 111;
        result += r1;
        continue;
        
    L6:
        r1 = g_volatile3 - 222;
        result += r1;
    }
    
    return result;
}

/* Helper function to create additional control flow complexity */
NOMIPS16 static int nested_condition_test(int x, int y) {
    register int a asm("t4") = x;
    register int b asm("t5") = y;
    register int c asm("t6") = 0;
    
    /* Multiple conditional jumps in sequence */
    if (a > b) {
        c = a - b;
        if (c > 10) {
            goto NESTED_L1;
        }
        return c + 1;
    }
    
    c = b - a;
    return c + 2;
    
NESTED_L1:
    /* Independent instruction after label */
    c = c * 2;
    return c + 3;
}

/* Main function that exercises the delay slot patterns */
NOMIPS16 int main(void) {
    int total = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Test with multiple iterations to ensure reorg pass is invoked */
    total += test_delay_slot_pattern(100);
    
    /* Additional test with different parameters */
    total += nested_condition_test(50, 30);
    total += nested_condition_test(10, 40);
    total += nested_condition_test(100, 20);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Verify with a simple checksum */
    if (total != 0) {
        printf("Test completed successfully.\n");
    } else {
        printf("Warning: Result is zero - check optimization\n");
    }
    
    return 0;
}
