/* Target: MIPS with delay slot filling */
/* Compile with: -O2 -march=mips1 -fno-schedule-insns -fno-schedule-insns2 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define MIPS_NODELAY __attribute__((nomips16))

/* Volatile memory for non-optimizable resource patterns */
static volatile int mem_array[256] = {0};

MIPS_NODELAY int main(void) {
    /* Explicit register variables to control allocation */
    register int r0 asm("t0") = 1;  /* Candidate delay slot reg */
    register int r1 asm("t1") = 2;  /* Independent reg for next_trial */
    register int r2 asm("t2") = 3;  /* Condition variable */
    register int r3 asm("t3") = 4;  /* Accumulator */
    register int r4 asm("t4") = 5;  /* Another independent reg */
    
    volatile int *vptr = &mem_array[0];
    int i, result = 0;
    
    /* Initialize memory with pattern */
    for (i = 0; i < 256; i++) {
        mem_array[i] = i * 3;
    }
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* --- Pattern 1: Simple conditional with independent ops --- */
        /* This creates a candidate for delay slot filling */
        r0 = r0 + 1;  /* Could be moved into delay slot */
        
        /* Simple conditional jump to label (should be simplejump_p) */
        if (r2 > 0) {
            /* Force goto to ensure label placement */
            goto label1;
        }
        
        /* Fall-through path */
        r3 = r3 + 10;
        continue;
        
    label1:
        /* Independent instruction after label (potential next_trial) */
        /* Uses different register set than r0 */
        r1 = r1 * 2;
        
        /* --- Pattern 2: Another conditional with volatile access --- */
        /* Volatile load creates specific resource pattern */
        r4 = *vptr;
        vptr++;
        
        if (r1 != r0) {  /* Different condition */
            goto label2;
        }
        
        r3 = r3 - 5;
        continue;
        
    label2:
        /* Another independent instruction */
        /* Uses memory location different from vptr's current target */
        mem_array[128] = r1 + r4;
        
        /* --- Pattern 3: Nested conditionals for multiple slots --- */
        r0 = r0 & 0xFF;  /* Bitwise operation - non-trapping */
        
        if (r3 < 1000) {
            if (r4 > 50) {  /* Nested to create multiple jumps */
                goto label3;
            }
        }
        
        r3 = r3 + 1;
        continue;
        
    label3:
        /* Simple arithmetic that doesn't reference r0's resources */
        r1 = r1 - r4;
        
        /* --- Pattern 4: Loop with array bounds check --- */
        /* Array access with bounds check */
        if (i < 256) {
            r2 = mem_array[i];  /* Load using index */
        }
        
        if (r2 % 2 == 0) {  /* Even check */
            goto label4;
        }
        
        r3 = r3 + 3;
        continue;
        
    label4:
        /* Independent store operation */
        /* Different memory location than previous accesses */
        static volatile int counter = 0;
        counter = r1;
        
        /* Accumulate results to prevent elimination */
        result += r0 + r1 + r3 + r4;
        
        /* Modify condition variable for next iteration */
        r2 = (r2 + 1) & 7;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Final values: r0=%d, r1=%d, r3=%d, r4=%d\n", r0, r1, r3, r4);
    
    return 0;
}
