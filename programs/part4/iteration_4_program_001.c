/* Target: MIPS with delay slots - triggers specific reorg.cc logic */
#include <stdio.h>

/* Prevent MIPS16 mode which might not have delay slots */
__attribute__((nomips16, noinline))
static int check_condition(int a, int b) {
    return a > b;
}

int main(void) {
    /* Volatile variables to prevent optimization and create memory ops */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    volatile int mem1 = 0, mem2 = 0;
    
    /* Explicit register variables to control resource allocation */
    register int r1 asm("t0") = 10;  /* Candidate delay slot reg */
    register int r2 asm("t1") = 20;  /* Independent reg for next_trial */
    register int r3 asm("t2") = 30;  /* Another independent reg */
    register int r4 asm("t3") = 40;
    
    int result = 0;
    int i;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 100; i++) {
        /* ----- BLOCK 1: Simple conditional with arithmetic before jump ----- */
        /* This arithmetic could be moved into delay slot */
        r1 = r1 + v1;  /* Uses t0, sets t0 */
        
        /* Simple conditional jump to label - should be simplejump_p */
        if (check_condition(r1, r2)) {
            /* Target label with independent instruction */
            /* This instruction (next_trial) uses different registers */
            goto label1;
        }
        /* Fall-through path */
        r3 = r3 - v2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses t2 (r3), doesn't touch t0 (r1) or conflict with r1 = r1 + v1 */
        r2 = r2 & 0xFF;  /* Simple arithmetic, no traps */
        result += 1;
        
        
        /* ----- BLOCK 2: Another pattern with different registers ----- */
        /* Different register set for this candidate */
        r4 = r4 | 0x55;  /* Uses t3 */
        
        /* Another simple conditional */
        if (r4 > r1) {
            goto label2;
        }
        r1 = r1 * 2;
        continue;
        
    label2:
        /* Independent instruction using yet another register */
        /* Simple load operation - volatile ensures it's not optimized away */
        mem1 = v3;  /* Memory op, doesn't use t3 or t0 */
        result += 2;
        
        
        /* ----- BLOCK 3: Pattern with volatile memory access ----- */
        /* Volatile store as delay slot candidate */
        v4 = r2;  /* Sets volatile memory, uses t1 */
        
        /* Conditional with different comparison */
        if (r3 != r4) {
            goto label3;
        }
        r2 = r2 + 5;
        continue;
        
    label3:
        /* Independent arithmetic after label */
        r3 = r3 ^ 0xAA;  /* Uses t2, independent of v4 = r2 */
        result += 3;
        
        
        /* ----- BLOCK 4: Multiple consecutive conditionals ----- */
        /* Creates multiple slots_to_fill opportunities */
        r1 = r1 << 1;  /* Candidate 1 */
        
        if (r1 < 1000) {
            goto label4a;
        }
        r4 = r4 >> 1;
        
    label4a:
        r2 = r2 + i;  /* This could be next_trial for first jump */
        result += 4;
        
        /* Second conditional in sequence */
        r3 = r3 - 1;  /* Candidate 2 */
        
        if (r2 > r3) {
            goto label4b;
        }
        r1 = r1 + 1;
        
    label4b:
        /* Simple non-trapping arithmetic */
        r4 = (r4 + 1) & 0x7FFFFFFF;  /* Ensure no overflow trap */
        result += 5;
    }
    
    /* Use all results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d\n", r1, r2, r3, r4);
    printf("Memory: mem1=%d, v4=%d\n", mem1, v4);
    
    return 0;
}
