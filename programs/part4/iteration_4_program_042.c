/* Target: MIPS with delay slots - triggers specific uncovered reorg.cc logic */
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
    
    /* Initialize registers with distinct values */
    r1 = 100;
    r2 = 200;
    r3 = 300;
    r4 = 400;
    r5 = 500;
    r6 = 600;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (i = 0; i < 1000; i++) {
        /* BLOCK 1: Creates a simple conditional jump with independent operations */
        /* This arithmetic could be a delay slot candidate */
        r1 = r1 + mem1;  /* Uses t0, references mem1 */
        
        /* Simple conditional jump - should compile to simplejump_p */
        if (r1 > 50) {
            /* Jump target label with independent instruction */
            goto label1;
        }
        
        /* Fall-through path */
        r2 = r2 - mem2;
        continue;
        
    label1:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (t2) and different memory (mem3) */
        r3 = r3 & mem3;  /* Simple arithmetic, non-trapping */
        result += r3;
        
        
        /* BLOCK 2: Another pattern with different registers */
        r4 = r4 ^ mem4;  /* Candidate for delay slot */
        
        if (r4 != 0) {   /* Another simple condition */
            goto label2;
        }
        
        r5 = r5 | mem1;
        continue;
        
    label2:
        /* Another independent instruction - different resources */
        r6 = r6 + 1;     /* Simple increment, no memory reference */
        result += r6;
        
        
        /* BLOCK 3: Pattern with memory operations */
        /* Load operation as delay slot candidate */
        int temp = mem1;
        
        if (temp < 100) {
            goto label3;
        }
        
        mem2 = temp;
        continue;
        
    label3:
        /* Store operation as next_trial - uses different memory location */
        mem3 = r2;       /* Different register (t1) and memory (mem3) */
        result += mem3;
        
        
        /* BLOCK 4: Arithmetic with constants */
        r1 = r1 * 2;
        
        /* Multiple simple conditions to increase slots_to_fill analysis */
        if (r1 > r2) {
            goto label4;
        }
        if (r1 == r2) {  /* Additional conditional jump */
            goto label5;
        }
        
        r2 = r2 / 2;
        continue;
        
    label4:
        /* Simple arithmetic with constant */
        r3 = r3 - 5;
        result += r3;
        continue;
        
    label5:
        /* Different arithmetic pattern */
        r4 = r4 % 7;
        result += r4;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Registers: t0=%d, t1=%d, t2=%d, t3=%d, t4=%d, t5=%d\n", 
           r1, r2, r3, r4, r5, r6);
    printf("Memory: %d %d %d %d\n", mem1, mem2, mem3, mem4);
    
    return 0;
}
