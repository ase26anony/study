/* Target: MIPS with delay slots (-march=mips1) */
#include <stdio.h>

/* Prevent MIPS16 mode which may not have delay slots */
#ifndef __mips__
#error "This program is designed for MIPS architecture"
#endif

/* Force use of specific registers to control resource allocation */
register int reg_a asm("$8");   /* t0 - for delay slot candidate */
register int reg_b asm("$9");   /* t1 - for next_trial instruction */
register int reg_c asm("$10");  /* t2 - for condition testing */
register int reg_d asm("$11");  /* t3 - for accumulation */

/* Volatile memory to create specific resource patterns */
volatile int mem1 = 1234;
volatile int mem2 = 5678;
volatile int mem3 = 9012;

int main(void) {
    int i, result = 0;
    
    /* Initialize register variables */
    reg_a = 100;
    reg_b = 200;
    reg_c = 0;
    reg_d = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        reg_c = mem1 + i;  /* This could be delay slot candidate */
        
        /* Conditional jump to label - should be simplejump_p */
        if (reg_c > 500) {
            /* Target label with independent instruction */
            __asm__ volatile ("" ::: "memory"); /* Memory barrier */
            target1:
            /* Independent arithmetic - potential next_trial */
            reg_b = reg_b + mem2;  /* Uses different register than reg_c */
            /* Avoid jump here - keep it non-jump */
            reg_d += reg_b;
            goto cont1;
        }
        goto cont1;
        
        cont1:
        
        /* Pattern 2: Another conditional jump pattern */
        reg_a = mem3 - i;  /* Another delay slot candidate */
        
        if (reg_a < 800) {
            __asm__ volatile ("" ::: "memory");
            target2:
            /* Independent operation using different resource */
            reg_b = reg_b & 0xFF;  /* Simple non-trapping operation */
            reg_d += reg_a;
            goto cont2;
        }
        goto cont2;
        
        cont2:
        
        /* Pattern 3: More complex to fill multiple slots */
        reg_c = i * 2;
        if (reg_c != 0) {
            __asm__ volatile ("" ::: "memory");
            target3:
            /* Another independent instruction */
            reg_b = reg_b | 0x100;
            reg_d += reg_c;
            goto cont3;
        }
        goto cont3;
        
        cont3:
        
        /* Pattern 4: Use volatile store/load to create memory ops */
        mem1 = i;
        if (mem2 > 3000) {
            __asm__ volatile ("" ::: "memory");
            target4:
            /* Independent memory operation */
            mem3 = reg_b + 1;  /* Different memory location than mem1 */
            reg_d += mem3;
            goto cont4;
        }
        goto cont4;
        
        cont4:
        
        /* Accumulate results to prevent elimination */
        result += reg_d;
        reg_d = 0;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: a=%d, b=%d, c=%d\n", reg_a, reg_b, reg_c);
    
    return 0;
}
