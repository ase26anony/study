#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific resource patterns */
    volatile int mem1 = 0x1234;
    volatile int mem2 = 0x5678;
    volatile int mem3 = 0x9ABC;
    volatile int mem4 = 0xDEF0;
    
    /* Register variables to control register allocation */
    register int r0 asm("t0") = 0;
    register int r1 asm("t1") = 1;
    register int r2 asm("t2") = 2;
    register int r3 asm("t3") = 3;
    register int r4 asm("t4") = 4;
    register int r5 asm("t5") = 5;
    register int r6 asm("t6") = 6;
    register int r7 asm("t7") = 7;
    
    int result = 0;
    int i;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        r0 = mem1 + i;          /* Potential delay slot candidate */
        if (r0 > 100) {         /* Should compile to simple conditional jump */
            /* Jump target label with independent instruction */
            target1:
            /* Independent arithmetic using different registers */
            r4 = r5 + r6;       /* next_trial candidate - doesn't reference r0 */
            result += r4;
        }
        
        /* Pattern 2: Another conditional jump pattern */
        r1 = mem2 - i;          /* Different register set */
        if (r1 < 50) {
            target2:
            /* Independent operation with volatile memory access */
            mem3 = r2 * r3;     /* Uses different registers than r1 */
            result += mem3;
        }
        
        /* Pattern 3: More complex but still simple jump */
        r2 = mem3 ^ i;
        if (r2 != 0) {
            target3:
            /* Simple arithmetic that cannot trap */
            r7 = r4 & 0xFF;     /* Mask operation - safe, non-trapping */
            result += r7;
        }
        
        /* Pattern 4: Using goto for explicit label jumps */
        r3 = mem4 | i;
        if (r3 > 2000) {
            goto target4;
        }
        /* Some code to skip over */
        r5 = r6 + 1;
        goto continue_loop;
        
        target4:
        /* Independent load operation */
        r6 = mem1;              /* Load from volatile - different resource */
        result += r6;
        
        continue_loop:
        /* Prevent loop unrolling for more reorg opportunities */
        asm volatile("" : : : "memory");
    }
    
    /* Pattern 5: Nested conditionals to create multiple jump opportunities */
    for (i = 0; i < 50; i++) {
        r0 = i * 2;
        if (r0 > 25) {
            if (r0 < 75) {
                target5:
                /* Simple arithmetic with constants */
                r1 = 100 - i;   /* Independent computation */
                result += r1;
            }
        }
        
        /* Pattern 6: Switch-like pattern with goto labels */
        r2 = i % 3;
        if (r2 == 0) {
            goto case0;
        } else if (r2 == 1) {
            goto case1;
        } else {
            goto case2;
        }
        
        case0:
        r3 = i + 10;
        result += r3;
        goto end_switch;
        
        case1:
        r4 = i * 3;
        result += r4;
        goto end_switch;
        
        case2:
        r5 = i / 2;
        result += r5;
        /* fall through */
        
        end_switch:
        /* Empty to test fall-through */
        ;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: %d %d %d %d %d %d %d %d\n", 
           r0, r1, r2, r3, r4, r5, r6, r7);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
