/* 
 * Program to trigger GCC's delay slot filling logic in reorg.cc
 * Specifically targets lines 2135-2149 for coverage
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2 -o reorg_test reorg_test.c
 * For coverage analysis: add -fdump-rtl-reorg -fdump-rtl-dbr
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which may not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Global volatile variables to create memory dependencies */
volatile int g_mem1 = 100;
volatile int g_mem2 = 200;
volatile int g_mem3 = 300;
volatile int g_mem4 = 400;

/* Function to create delay slot filling opportunities */
NOMIPS16 int test_delay_slots(int iterations) {
    /* Use explicit register variables to control resource allocation */
    register int r1 asm("t0") = 0;  /* Candidate for delay slot */
    register int r2 asm("t1") = 0;  /* Used in jump condition */
    register int r3 asm("t2") = 0;  /* Used after label (next_trial) */
    register int r4 asm("t3") = 0;  /* Another independent register */
    register int r5 asm("t4") = 0;  /* Yet another independent register */
    
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple arithmetic that could fill delay slot */
        r1 = g_mem1 + i;  /* This could be moved into delay slot */
        
        /* Conditional jump to label - should be simplejump_p */
        if (r2 > 0) {
            /* Target label must be immediately after usable instructions */
            goto label1;
        }
        
        /* Some code to avoid fall-through optimization */
        r4 = r4 ^ 0x55;
        continue;
        
    label1:
        /* Instruction after label (next_trial) - must be eligible for moving */
        /* Uses different register (r3) than delay slot candidate (r1) */
        r3 = g_mem2 & 0xFF;  /* Simple, non-trapping operation */
        
        /* Accumulate result to prevent elimination */
        result += r1 + r3;
        
        /* Pattern 2: Another similar pattern with different registers */
        r5 = g_mem3 - i;
        
        if (r4 != 0) {
            goto label2;
        }
        
        r2 = r2 | 0xAA;
        continue;
        
    label2:
        /* Another independent instruction after label */
        r1 = g_mem4 >> 2;  /* Simple shift, won't trap */
        result += r5 * r1;
        
        /* Pattern 3: Nested condition to create more opportunities */
        r2 = i * 2;
        r3 = g_mem1 + 5;
        
        if (r2 < 100) {
            goto label3;
        }
        
        r4 = r4 + 1;
        continue;
        
    label3:
        /* Instruction uses yet another register to avoid conflicts */
        r4 = r3 & 0x0F;  /* Independent of r2 */
        result += r2 - r4;
        
        /* Update condition registers for next iteration */
        r2 = result & 1;
        r4 = i & 3;
    }
    
    return result;
}

/* Another function with different patterns */
NOMIPS16 int more_delay_opportunities(int seed) {
    register int a asm("s0") = seed;
    register int b asm("s1") = seed * 2;
    register int c asm("s2") = 0;
    register int d asm("s3") = 0;
    
    int total = 0;
    
    /* Multiple jumps in sequence to increase slots_to_fill analysis */
    if (a > 100) {
        c = g_mem1 + a;  /* Potential delay slot filler */
        goto jump_a;
    }
    
    b = b ^ 0x1234;
    
jump_a:
    /* Independent instruction after label */
    d = g_mem2 | 0x80;
    total += c + d;
    
    /* Another jump immediately after */
    if (b < 50) {
        a = g_mem3 - b;
        goto jump_b;
    }
    
    c = c << 1;
    
jump_b:
    /* Another independent instruction */
    b = g_mem4 & 0x7F;
    total += a * b;
    
    /* One more to create a chain */
    if (total != 0) {
        d = total + 1;
        goto jump_c;
    }
    
    a = a >> 2;
    
jump_c:
    c = d - 10;
    total += c;
    
    return total;
}

int main() {
    int result1, result2;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Call with enough iterations to trigger reorg pass */
    result1 = test_delay_slots(1000);
    result2 = more_delay_opportunities(42);
    
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Use results to prevent dead code elimination */
    if ((result1 + result2) > 0) {
        printf("Test completed successfully.\n");
    } else {
        printf("Unexpected result.\n");
    }
    
    return 0;
}
