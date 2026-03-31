/* reorg_delay_slot_test.c
 * Designed to trigger GCC's delay slot filling logic in reorg.cc lines 2135-2149
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 * Or for QEMU: mips-linux-gnu-gcc -O2 -march=mips1 -static -o test test.c && qemu-mips ./test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Use explicit register variables to control resource allocation */
register int r0 asm("t0");  /* Candidate delay slot register */
register int r1 asm("t1");  /* Independent register for next_trial */
register int r2 asm("t2");  /* Another independent register */
register int r3 asm("t3");  /* Loop counter register */

/* Volatile variables to prevent optimization and create specific patterns */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;
volatile int* volatile mem_ptr = (int*)0x1000;

int main() NOMIPS16;

int main() NOMIPS16
{
    int result = 0;
    int i;
    
    /* Initialize register variables */
    r0 = 0;
    r1 = 0;
    r2 = 0;
    r3 = 0;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 100; i++) {
        /* BLOCK 1: Simple conditional with arithmetic before jump */
        /* This creates a delay slot candidate (insn) */
        r0 = v1 + v2;  /* Potential delay slot instruction */
        
        /* Conditional jump to label L1 - should be simplejump_p */
        if (r0 > 500) {
            goto L1;
        }
        
        /* Some code to avoid fall-through optimization */
        r2 = r2 + 1;
        continue;
        
    L1:
        /* This is next_trial: independent arithmetic that doesn't reference
           resources used by r0 calculation above */
        r1 = v3 & 0xFF;  /* Independent operation - eligible for delay slot */
        result += r1;
        
        /* BLOCK 2: Another conditional with different registers */
        r3 = v2 - v1;  /* Another delay slot candidate */
        
        if (r3 < 200) {
            goto L2;
        }
        
        r2 = r2 + 2;
        continue;
        
    L2:
        /* Another independent next_trial candidate */
        r1 = (v1 * 2) | 0x1;
        result += r1;
        
        /* BLOCK 3: More complex to increase slots_to_fill analysis */
        /* Use volatile memory access to create specific resource pattern */
        int temp = *mem_ptr;  /* Volatile load - can't be optimized away */
        r0 = temp + i;
        
        if (r0 != 0) {
            goto L3;
        }
        
        r2 = r2 + 3;
        continue;
        
    L3:
        /* Independent arithmetic with no resource conflicts */
        r1 = (i << 2) + 5;
        result += r1;
        
        /* BLOCK 4: Nested conditionals to create multiple jump opportunities */
        r2 = v3 / 2;
        
        if (r2 > 100) {
            if (r1 < 50) {
                goto L4;
            }
        }
        
        r2 = r2 + 4;
        continue;
        
    L4:
        /* Simple arithmetic - not a jump, not a SEQUENCE, can't throw */
        r1 = r1 ^ 0x55;  /* XOR is safe and independent */
        result += r1;
        
        /* BLOCK 5: Final block with register pressure */
        /* Force use of different register sets */
        register int r4 asm("t4") = result;
        register int r5 asm("t5") = i;
        
        r0 = r4 * r5;
        
        if (r0 % 2 == 0) {
            goto L5;
        }
        
        r2 = r2 + 5;
        continue;
        
    L5:
        /* Final independent instruction - carefully crafted to pass all checks:
           - Not a jump
           - Not a SEQUENCE  
           - Doesn't reference/set conflicting resources
           - Can't throw (integer arithmetic)
           - Eligible for delay slot */
        r1 = (r5 & 0xF) + 1;
        result += r1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("r0: %d, r1: %d, r2: %d, r3: %d\n", r0, r1, r2, r3);
    
    /* Additional volatile operations to ensure reorg sees them */
    v1 = result;
    v2 = r0;
    v3 = r1;
    
    return 0;
}
