/* Program to trigger GCC delay slot filling logic in reorg.cc lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
#ifdef __mips__
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")
#elif __sparc__
#define REG1 asm("%l0")
#define REG2 asm("%l1")
#define REG3 asm("%l2")
#define REG4 asm("%l3")
#define REG5 asm("%l4")
#define REG6 asm("%l5")
#else
/* Generic register hints for RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main() {
    /* Use register variables to control allocation and avoid resource conflicts */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 1;
    register int e REG5 = 2;
    register int f REG6 = 3;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    int result = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch conditions to create different execution paths */
        a = i * 3;
        b = 100 - i;
        
        /* BRANCH 1: Conditional branch with predictable pattern */
        /* This creates a simplejump_p that jumps to label1 */
        if (__builtin_expect(a > b, 0)) {
            /* Force a nop that could be replaced by delay slot filling */
            asm volatile("nop" ::: "memory");
            goto label1;
        }
        
        /* Some filler code to separate branches */
        c = d + e;
        
        /* BRANCH 2: Another conditional branch with different pattern */
        /* This creates another simplejump_p opportunity */
        if (__builtin_expect((i & 1) == 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slots */
            goto label2;
        }
        
        /* More filler to create distance */
        f = e - d;
        
        /* BRANCH 3: Third pattern with different register usage */
        if (__builtin_expect(c < f, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
            goto label3;
        }
        
        /* Continue loop if no branch taken */
        result += i;
        continue;
        
    /* TARGET LABELS with simple arithmetic instructions */
    /* These instructions are eligible for delay slot filling:
       - Non-jump, non-sequence instructions
       - No resource conflicts with branches
       - No traps/faults
       - Simple register operations */
    label1:
        /* Simple arithmetic using registers not involved in branch condition */
        /* a and b were used in branch condition, so use d and e instead */
        d = e + 1;  /* Eligible delay slot candidate */
        result += 1;
        continue;
        
    label2:
        /* Another simple arithmetic instruction */
        e = f + 2;  /* Eligible delay slot candidate */
        result += 2;
        continue;
        
    label3:
        /* More simple arithmetic */
        f = d + 3;  /* Eligible delay slot candidate */
        result += 3;
        continue;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test case: nested loops with varying complexity */
    {
        register int x REG1 = 0;
        register int y REG2 = 50;
        register int z REG3 = 0;
        register int w REG4 = 10;
        
        for (int j = 0; j < 50; j++) {
            x = j * 2;
            y = 100 - j;
            
            /* Branch with immediate label following */
            if (__builtin_expect(x < y, 1)) {
                asm volatile("nop" ::: "memory");
                goto target_label;
            }
            
            z = w * 2;
            continue;
            
        target_label:
            /* Perfect delay slot candidate: simple operation with no
               resource conflicts or side effects */
            w = z + 5;  /* Uses z (not used in branch) and constant */
            /* Continue loop */
        }
        
        printf("Secondary result: %d\n", w);
    }
    
    return result > 0 ? 0 : 1;
}

/* Helper function to create more complex control flow */
static void create_branch_patterns(int limit) {
    register int p asm("r8") = 0;
    register int q asm("r9") = 10;
    register int r asm("r10") = 20;
    register int s asm("r11") = 30;
    
    for (int k = 0; k < limit; k++) {
        p = k;
        
        /* Multiple branches in sequence to give delay slot filler
           multiple trials to consider */
        if (__builtin_expect(p < 5, 0)) {
            asm volatile("nop" ::: "memory");
            goto pat1;
        }
        
        if (__builtin_expect(p > 10, 0)) {
            asm volatile("nop" ::: "memory");
            goto pat2;
        }
        
        r = s - p;
        continue;
        
    pat1:
        /* Candidate instruction after label */
        q = r + 1;  /* Safe for delay slot */
        continue;
        
    pat2:
        /* Another candidate */
        s = q + 2;  /* Safe for delay slot */
        continue;
    }
}
