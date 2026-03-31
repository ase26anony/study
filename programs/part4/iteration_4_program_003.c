#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16))
int main() {
    /* Volatile variables to prevent optimization and create specific memory patterns */
    volatile int mem1 = 1;
    volatile int mem2 = 2;
    volatile int mem3 = 3;
    volatile int mem4 = 4;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0");
    register int r2 asm("t1");
    register int r3 asm("t2");
    register int r4 asm("t3");
    register int r5 asm("t4");
    register int r6 asm("t5");
    
    int result = 0;
    int i;
    
    /* Initialize registers */
    r1 = 100;
    r2 = 200;
    r3 = 300;
    r4 = 400;
    r5 = 500;
    r6 = 600;
    
    /* Loop to increase reorg pass analysis opportunities */
    for (i = 0; i < 1000; i++) {
        /* Pattern 1: Simple conditional jump with arithmetic before */
        /* This could be a delay slot candidate */
        r1 = r1 + mem1;  /* Uses r1 and volatile mem1 */
        
        /* Conditional jump to label L1 */
        if (r1 > 50) {
            goto L1;
        }
        
        /* Alternative path to avoid infinite loops */
        r1 = r1 - 1;
        continue;
        
    L1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r2) and different volatile memory */
        r2 = r2 & mem2;  /* Simple bitwise AND, non-trapping */
        
        /* Accumulate result to prevent dead code elimination */
        result += r2;
        
        
        /* Pattern 2: Another conditional jump with different registers */
        r3 = r3 - mem3;  /* Delay slot candidate using r3 */
        
        if (r3 < 1000) {
            goto L2;
        }
        
        r3 = r3 + 1;
        continue;
        
    L2:
        /* Another independent instruction after label */
        r4 = r4 | mem4;  /* Bitwise OR, non-trapping */
        result += r4;
        
        
        /* Pattern 3: More complex to increase slots_to_fill analysis */
        /* Multiple volatile accesses to create resource patterns */
        int temp = mem1 + mem2;
        r5 = r5 ^ temp;  /* XOR operation */
        
        if (r5 != 0) {
            goto L3;
        }
        
        r5 = r5 + 1;
        continue;
        
    L3:
        /* Simple arithmetic that doesn't reference r5's resources */
        r6 = r6 * 2;  /* Multiplication by 2 is simple shift */
        result += r6;
        
        
        /* Pattern 4: Nested control flow to create multiple jump opportunities */
        r1 = r1 + i;  /* Modify r1 again */
        
        if (r1 % 2 == 0) {
            goto L4;
        }
        
        /* Alternate path with another conditional */
        if (r2 > 100) {
            goto L5;
        }
        
        continue;
        
    L4:
        /* Independent instruction using yet another operation */
        r3 = r3 >> 1;  /* Shift right */
        result += r3;
        continue;
        
    L5:
        r4 = r4 << 1;  /* Shift left */
        result += r4;
        
        
        /* Pattern 5: Memory operations with volatile */
        /* Load from volatile memory - creates specific resource pattern */
        int load_val = mem1;
        r2 = r2 + load_val;
        
        if (r2 < 500) {
            goto L6;
        }
        
        r2 = r2 - 10;
        continue;
        
    L6:
        /* Store to volatile memory - different resource */
        mem3 = r6;  /* Uses r6, not r2 */
        result += mem3;
    }
    
    printf("Result: %d\n", result);
    printf("Final registers: %d %d %d %d %d %d\n", r1, r2, r3, r4, r5, r6);
    
    return 0;
}
