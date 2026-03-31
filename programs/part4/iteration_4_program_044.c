/* Target: MIPS with delay slots (-march=mips1) */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
#ifndef __mips__
#error "This program is designed for MIPS architecture"
#endif

/* Use register variables to control resource allocation */
register int reg_a asm("$8");   /* t0 */
register int reg_b asm("$9");   /* t1 */
register int reg_c asm("$10");  /* t2 */
register int reg_d asm("$11");  /* t3 */
register int reg_e asm("$12");  /* t4 */
register int reg_f asm("$13");  /* t5 */

/* Volatile memory to create specific resource patterns */
volatile int mem1 = 123;
volatile int mem2 = 456;
volatile int mem3 = 789;

/* Global accumulator to prevent optimization */
int global_acc = 0;

/* Function to create delay slot filling opportunities */
void create_delay_slot_opportunities(int iterations) {
    /* Local volatile variables for memory operations */
    volatile int local_mem = 0;
    volatile int local_temp = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Setup different register values each iteration */
        reg_a = i * 3;
        reg_b = i * 5;
        reg_c = i * 7;
        reg_d = i * 11;
        reg_e = i * 13;
        reg_f = i * 17;
        
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This could be a delay slot candidate */
        reg_a = reg_b + reg_c;  /* Potential delay slot instruction */
        
        /* Conditional jump to label L1 */
        if (reg_a > 100) {
            /* Jump target L1 - next_trial candidate */
            L1:
            /* Independent instruction using different registers */
            reg_d = reg_e & reg_f;  /* Should not conflict with reg_a */
            global_acc += reg_d;
            /* Continue after label */
        }
        
        /* Pattern 2: Another conditional with different resources */
        /* Use memory operations to create resource patterns */
        local_temp = mem1;
        reg_b = local_temp * 2;  /* Potential delay slot */
        
        if (reg_b < 500) {
            L2:
            /* Independent arithmetic with different register */
            reg_c = reg_d | 0xFF;  /* Different operation, different reg */
            global_acc += reg_c;
        }
        
        /* Pattern 3: Nested conditionals for multiple slots analysis */
        reg_e = mem2 + i;
        if (reg_e != 0) {
            L3:
            /* Simple arithmetic that doesn't trap */
            reg_f = (reg_a << 2) + 1;  /* Shift and add - safe */
            global_acc += reg_f;
            
            /* Inner conditional for more complexity */
            if (reg_f > 50) {
                L4:
                /* Another independent operation */
                reg_a = reg_b ^ reg_c;  /* XOR - different registers */
                global_acc += reg_a;
            }
        }
        
        /* Pattern 4: Goto with explicit label placement */
        reg_d = mem3 - i;
        if (reg_d % 2 == 0) {
            goto L5;
        }
        /* Skip over the label to create jump-to-label pattern */
        reg_e = 999;
        goto L6;
        
        L5:
        /* Instruction after label - potential next_trial */
        reg_f = reg_a * 3;  /* Uses reg_a which was set earlier */
        global_acc += reg_f;
        
        L6:
        /* Continue loop */
        ;
        
        /* Pattern 5: Multiple consecutive conditionals */
        /* This increases slots_to_fill analysis opportunities */
        local_temp = i & 0xF;
        reg_a = local_temp * 10;
        
        if (reg_a < 100) goto L7;
        if (reg_a > 200) goto L8;
        
        L7:
        /* Simple non-trapping arithmetic */
        reg_b = reg_c + 42;  /* Different register from condition */
        global_acc += reg_b;
        goto L9;
        
        L8:
        reg_d = reg_e - 24;  /* Another independent operation */
        global_acc += reg_d;
        
        L9:
        /* Loop continues */
    }
}

/* Main function with compilation hints */
int main() {
    /* Initialize volatile memory */
    mem1 = 1000;
    mem2 = 2000;
    mem3 = 3000;
    
    /* Call function multiple times to ensure reorg pass runs */
    create_delay_slot_opportunities(100);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", global_acc);
    
    /* Additional test with different optimization patterns */
    global_acc = 0;
    
    /* Test with register pressure */
    {
        register int r1 asm("$16");  /* s0 */
        register int r2 asm("$17");  /* s1 */
        register int r3 asm("$18");  /* s2 */
        
        r1 = 1; r2 = 2; r3 = 3;
        
        for (int i = 0; i < 50; i++) {
            /* Create resource patterns for conflict analysis */
            r1 = r1 * 3 + i;
            
            if (r1 > 1000) {
                L10:
                /* Independent operation with different register set */
                r2 = r3 << (i & 3);  /* Non-trapping shift */
                global_acc += r2;
            }
            
            /* Another pattern */
            r3 = mem1 + r2;
            if (r3 < 500) {
                L11:
                r1 = r2 ^ i;  /* XOR - safe operation */
                global_acc += r1;
            }
        }
    }
    
    printf("Final result: %d\n", global_acc);
    return 0;
}
