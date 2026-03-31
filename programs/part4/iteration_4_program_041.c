/* reorg_coverage.c - Target MIPS delay slot filling for uncovered reorg.cc lines */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile memory locations to create specific resource patterns */
volatile int mem1 = 100;
volatile int mem2 = 200;
volatile int mem3 = 300;
volatile int mem4 = 400;

/* Global accumulator to prevent dead code elimination */
int global_acc = 0;

/* Function with tight control flow for delay slot analysis */
NOMIPS16 void delay_slot_test(int iterations) {
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Initialize registers with volatile memory values */
        r1 = mem1 + i;
        r2 = mem2 - i;
        r3 = mem3 * (i + 1);
        r4 = mem4;
        r5 = 0;
        r6 = 0;
        
        /* PATTERN 1: Simple conditional jump with arithmetic before */
        /* This creates a delay slot candidate (the add) */
        r1 = r1 + r2;  /* Candidate for delay slot filling */
        
        /* Conditional jump to label L1 */
        if (r1 > 0) {
            /* Jump target L1 - next_trial should be independent */
            L1:
            /* Independent arithmetic using different registers */
            r3 = r3 & 0xFF;  /* Simple, non-trapping operation */
            /* Continue execution */
            r5 = r5 + r3;
        }
        
        /* PATTERN 2: Another conditional jump pattern */
        r4 = r4 - 1;  /* Another delay slot candidate */
        
        if (r4 != 0) {
            L2:
            /* Independent operation after label */
            r6 = r6 | 0x55;  /* Simple bitwise operation */
            r5 = r5 + r6;
        }
        
        /* PATTERN 3: goto with label immediately after */
        r2 = r2 * 2;  /* Delay slot candidate */
        
        if (r2 < 1000) {
            goto L3;
        }
        
        /* Some code to skip over */
        r1 = r1 + 1;
        
        L3:
        /* Instruction after label - potential next_trial */
        r3 = r3 + 5;  /* Simple arithmetic, no traps */
        r5 = r5 + r3;
        
        /* PATTERN 4: Nested control flow */
        r1 = r1 & 0xF;  /* Delay slot candidate */
        
        if (r1 == 0) {
            L4:
            /* Independent operation */
            r4 = r4 ^ 0xAA;  /* Simple XOR */
            r5 = r5 + r4;
        } else {
            /* Alternative path */
            r6 = r6 + 10;
        }
        
        /* PATTERN 5: Multiple consecutive conditionals */
        r3 = r3 - 2;  /* Candidate 1 */
        if (r3 > 50) {
            L5a:
            r1 = r1 << 1;  /* Potential next_trial */
            r5 = r5 + r1;
        }
        
        r4 = r4 + 3;  /* Candidate 2 */
        if (r4 < 500) {
            L5b:
            r2 = r2 >> 1;  /* Potential next_trial */
            r5 = r5 + r2;
        }
        
        /* Accumulate results to prevent elimination */
        global_acc = global_acc + r5 + r1 + r2 + r3 + r4 + r6;
        
        /* Force register reload from volatile memory */
        /* This creates resource patterns the reorg pass must analyze */
        asm volatile("" : : "r"(mem1), "r"(mem2), "r"(mem3), "r"(mem4));
    }
}

/* Helper to create more complex control flow */
NOMIPS16 int conditional_helper(int x, int y) {
    register int a asm("t6");
    register int b asm("t7");
    register int c asm("t8");
    
    a = x;
    b = y;
    
    /* Multiple conditionals in sequence */
    if (a > b) {
        L_helper1:
        c = a - b;  /* Independent after label */
        return c;
    }
    
    if (a < b) {
        L_helper2:
        c = b - a;  /* Independent after label */
        return c;
    }
    
    L_helper3:
    c = a + b;  /* Independent after label */
    return c;
}

/* Main function with compilation guard for MIPS */
#ifdef __mips__
int main() {
    int i, result;
    
    printf("Testing MIPS delay slot filling patterns\n");
    
    /* Call the test function multiple times */
    for (i = 0; i < 10; i++) {
        delay_slot_test(100);
        result = conditional_helper(i * 10, i * 5);
        global_acc += result;
    }
    
    printf("Accumulated result: %d\n", global_acc);
    printf("Test completed\n");
    
    return 0;
}
#else
int main() {
    printf("This program is designed for MIPS architecture with delay slots\n");
    printf("Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -mabi=32\n");
    return 0;
}
#endif
