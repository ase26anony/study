/* reorg_coverage.c
 * Target: MIPS with delay slots (-march=mips1)
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-reorg -o reorg_coverage reorg_coverage.c
 * Run with QEMU: qemu-mips ./reorg_coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Function to create delay slot filling opportunities */
NOMIPS16 static int create_delay_slot_opportunities(int iterations) {
    /* Use explicit register variables to control register allocation */
    register int r1 asm("t0");  /* Candidate for delay slot */
    register int r2 asm("t1");  /* Used in next_trial */
    register int r3 asm("t2");  /* Another independent register */
    register int r4 asm("t3");  /* Yet another independent register */
    
    int result = 0;
    int i;
    
    /* Initialize registers with distinct values */
    r1 = 1;
    r2 = 2;
    r3 = 3;
    r4 = 4;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This arithmetic could be moved into delay slot */
        r1 = r1 + i;  /* Candidate for delay slot filling */
        
        /* Conditional jump to label L1 */
        if (r1 > 0) {
            /* Target: L1 */
            goto L1;
        }
        
        /* Fall-through path */
        r3 = r3 * 2;
        continue;
        
    L1:
        /* Instruction after label - potential next_trial */
        /* Uses different registers than r1 to avoid conflicts */
        r2 = r2 + g_volatile_array[i & 0xF];  /* Independent operation */
        result += r2;
        
        /* Pattern 2: Another conditional jump with different registers */
        r4 = r4 - i;  /* Another delay slot candidate */
        
        if (r4 != 0) {
            goto L2;
        }
        
        r1 = r1 | 0xFF;
        continue;
        
    L2:
        /* Another independent instruction after label */
        r3 = r3 & 0x7F;  /* Simple arithmetic, won't trap */
        result += r3;
        
        /* Pattern 3: Nested conditional jumps to create multiple opportunities */
        g_volatile_counter++;
        
        if (g_volatile_counter & 1) {
            r1 = r1 << 1;  /* Delay slot candidate */
            
            if (r2 < 100) {
                goto L3;
            }
            
            r4 = r4 >> 1;
            continue;
            
        L3:
            /* Independent arithmetic after label */
            r2 = r2 ^ 0x55;  /* Won't trap, uses different register */
            result += r2;
        }
        
        /* Pattern 4: Loop with volatile memory access */
        volatile int* ptr = &g_volatile_array[i & 0xF];
        int temp = *ptr;  /* Load - could be delay slot candidate */
        
        if (temp > 0) {
            goto L4;
        }
        
        r3 = r3 + 1;
        continue;
        
    L4:
        /* Simple arithmetic after label */
        r4 = r4 * 3;  /* Independent operation */
        result += r4;
        
        /* Prevent optimization by using all registers */
        g_volatile_array[i & 0xF] = r1 + r2 + r3 + r4;
    }
    
    /* Mix all results to prevent dead code elimination */
    return result + r1 + r2 + r3 + r4;
}

/* Helper function with more complex control flow */
NOMIPS16 static int nested_control_flow(int x) {
    register int a asm("t4");
    register int b asm("t5");
    register int c asm("t6");
    
    a = x;
    b = x * 2;
    c = x * 3;
    
    /* Series of conditional jumps */
    if (a > 10) {
        a = a + 5;  /* Delay slot candidate */
        goto label_a;
    }
    
    b = b - 2;
    return a + b + c;
    
label_a:
    /* Independent instruction after label */
    c = c & 0x3F;  /* Simple bitwise operation */
    
    if (b < 20) {
        b = b * 2;  /* Another candidate */
        goto label_b;
    }
    
    a = a | 0x80;
    return a + b + c;
    
label_b:
    /* Another independent instruction */
    a = a ^ 0xAA;  /* Won't trap */
    
    return a + b + c;
}

/* Main function to drive execution */
int main(void) {
    int total = 0;
    int i;
    
    printf("Starting delay slot coverage test...\n");
    
    /* Initialize volatile array with non-zero values */
    for (i = 0; i < 16; i++) {
        g_volatile_array[i] = i + 1;
    }
    
    /* Run multiple iterations to ensure reorg pass is triggered */
    total += create_delay_slot_opportunities(100);
    total += nested_control_flow(15);
    total += nested_control_flow(25);
    
    /* More iterations with different patterns */
    for (i = 0; i < 50; i++) {
        register int r asm("t7") = i;
        
        /* Simple if-goto pattern */
        if (r & 1) {
            r = r + g_volatile_counter;  /* Candidate */
            goto small_label;
        }
        
        r = r - 1;
        continue;
        
    small_label:
        /* Independent arithmetic */
        r = r * 2;  /* next_trial candidate */
        total += r;
    }
    
    printf("Result: %d\n", total);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
