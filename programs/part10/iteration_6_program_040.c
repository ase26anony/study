/* Program to trigger GCC's delay slot filling logic for RISC architectures */
#include <stdio.h>
#include <stdlib.h>

/* Force register usage for better control over resource conflicts */
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
/* Generic register hints for other RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main() {
    /* Declare variables in registers to avoid memory operations */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 1;
    register int e REG5 = 2;
    register int f REG6 = 3;
    
    /* Volatile iteration counter to prevent loop unrolling */
    volatile int iterations = 100;
    int i;
    
    /* Main loop with multiple conditional branches */
    for (i = 0; i < iterations; i++) {
        /* First branch: creates opportunity for delay slot filling */
        if (__builtin_expect(a < b, 1)) {
            /* Insert nop that might be replaced by delay slot filler */
            asm volatile("nop" ::: "memory");
            /* Target label 1 - simple arithmetic that doesn't trap */
            target1:
            /* Independent operation using different registers than branch condition */
            e = f + 1;  /* Simple, trap-free arithmetic */
        }
        
        /* Update variables to change branch behavior */
        a++;
        if (i % 3 == 0) {
            b--;
        }
        
        /* Second branch with different register usage pattern */
        if (__builtin_expect(c < d, 0)) {
            /* Multiple nops to give filler more candidates */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target2:
            /* Another simple, independent operation */
            f = e + 2;  /* Register-to-register operation, no traps */
        }
        
        /* Third branch with more complex pattern */
        if (__builtin_expect((a & 1) == 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target3:
            /* Safe arithmetic with immediate constant */
            c = d + 5;  /* Constant addition, cannot trap */
        }
        
        /* Update more variables */
        c = (c + 1) & 0xFF;
        d = (d * 2) & 0xFF;
        
        /* Fourth branch - mixed register usage */
        if (__builtin_expect(e > f, 0)) {
            /* Single nop for this branch */
            asm volatile("nop" ::: "memory");
            target4:
            /* Simple move-like operation */
            a = b + 0;  /* Effectively a = b, but expressed as addition */
        }
        
        /* Ensure all variables stay live and distinct */
        e = (e + f) & 0xFF;
        f = (f - 1) & 0xFF;
    }
    
    /* Final computation using all variables to keep them live */
    int result = (a + b + c + d + e + f) & 0xFFFF;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Additional function to create more branching opportunities */
static int helper(int x, int y) {
    register int r1 REG1 = x;
    register int r2 REG2 = y;
    register int r3 REG3 = 0;
    register int r4 REG4 = 0;
    
    /* Nested conditional with inline asm nops */
    if (__builtin_expect(r1 > r2, 1)) {
        asm volatile("nop" ::: "memory");
        helper_label1:
        r3 = r4 + 10;  /* Candidate for delay slot */
    } else {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        helper_label2:
        r4 = r3 + 20;  /* Another candidate */
    }
    
    /* Loop with variable iterations */
    for (int i = 0; i < (x & 0x3); i++) {
        if (__builtin_expect((r1 + i) < r2, 0)) {
            asm volatile("nop" ::: "memory");
            helper_label3:
            r1 = r2 + i;  /* Arithmetic with loop variable */
        }
    }
    
    return r1 + r2 + r3 + r4;
}
