/* reorg_delay_slot_test.c
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 * For coverage: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main(void) {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 100;
    volatile int mem2 = 200;
    volatile int mem3 = 300;
    volatile int mem4 = 400;
    
    /* Register variables to control register allocation */
    register int r1 asm("t0") = 1;  /* $t0 */
    register int r2 asm("t1") = 2;  /* $t1 */
    register int r3 asm("t2") = 3;  /* $t2 */
    register int r4 asm("t3") = 4;  /* $t3 */
    register int r5 asm("t4") = 5;  /* $t4 */
    register int r6 asm("t5") = 6;  /* $t6 */
    
    int result = 0;
    int i;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with independent instruction after label */
        /* This creates a simplejump_p to a label */
        if (r1 > 0) {
            /* Candidate for delay slot: uses r2, sets r1 */
            r1 = r2 + mem1;  /* This could be in delay slot */
            goto label1;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = r4 * 2;
        
    label1:
        /* next_trial candidate: independent instruction using different registers */
        /* Uses r5, sets r6 - doesn't conflict with r1/r2 above */
        r6 = r5 - mem2;  /* This could be moved into delay slot */
        result += r6;
        
        /* Pattern 2: Another conditional with different registers */
        if (r3 < 10) {
            /* Delay slot candidate: memory load to register */
            r2 = mem3;  /* Load operation */
            goto label2;
        }
        
        r4 = r1 | 0xFF;
        
    label2:
        /* next_trial: arithmetic with immediate */
        r5 = r4 & 0x0F;  /* Simple bit operation, won't trap */
        result += r5;
        
        /* Pattern 3: More complex condition but still simple jump */
        if ((r1 & 1) == 0) {
            /* Multiple operations to create scheduling opportunities */
            r3 = r6 << 2;
            goto label3;
        }
        
        r2 = r5 + 1;
        
    label3:
        /* Independent operation using volatile memory */
        mem4 = r2 + r3;  /* Store operation */
        result += mem4;
        
        /* Pattern 4: Nested condition to create multiple jump opportunities */
        if (i % 3 == 0) {
            if (r4 > r5) {
                r1 = r3 * r2;  /* Multiplication */
                goto label4;
            }
            r6 = r1 / 2;  /* Division might create complex patterns */
        }
        
        r4 = i & 0x7F;
        
    label4:
        /* Simple arithmetic that doesn't reference previous resources */
        r5 = r6 ^ 0x55;  /* XOR operation */
        result += r5;
        
        /* Modify registers to change conditions */
        r1 += i;
        r2 -= 1;
        r3 = (r3 << 1) | 1;
        r4 = ~r4;
        
        /* Access volatiles to create memory dependencies */
        mem1 += 1;
        mem2 -= 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d\n", r1, r2, r3, r4, r5, r6);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return result != 0;
}
