/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force register usage for predictable resource allocation */
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
    /* Declare variables in registers to avoid memory operations */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 1;
    register int e REG5 = 2;
    register int f REG6 = 3;
    
    /* Volatile counter to prevent loop unrolling */
    volatile int iterations = 100;
    int i;
    
    /* Result accumulator to keep computations live */
    int result = 0;
    
    /* Loop with multiple branches to create delay slot filling opportunities */
    for (i = 0; i < iterations; i++) {
        /* BRANCH 1: Simple conditional with predictable pattern */
        /* Create branch condition using separate registers */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
            /* Target label for branch - instruction here should be eligible */
            target1:
            /* Simple arithmetic operation - safe for delay slot */
            /* Uses registers not involved in branch condition */
            c = d + e;  /* This should be the 'next_trial' candidate */
        }
        
        /* Update variables to change branch patterns */
        a = a + 1;
        b = b - 1;
        result += c;
        
        /* BRANCH 2: Different condition to create another trial */
        /* Use different registers for condition */
        if (__builtin_expect(d < f, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slots */
            target2:
            /* Another simple operation with different registers */
            e = f + 1;  /* Another eligible candidate */
        }
        
        /* More variable updates */
        d = d + 2;
        f = f + 1;
        result += e;
        
        /* BRANCH 3: Nested condition for complex flow */
        if (__builtin_expect((a & 1) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            target3:
            /* Register move operation - very simple candidate */
            f = e;  /* Simple move, no traps, no conflicts */
        } else {
            /* Alternate path to create more complex CFG */
            asm volatile("nop" ::: "memory");
            target4:
            /* Another simple arithmetic */
            d = c + 2;
        }
        
        /* Continue updating to prevent dead code elimination */
        result += f + d;
        
        /* BRANCH 4: Loop-dependent condition */
        if (__builtin_expect(i < iterations/2, 1)) {
            /* Multiple nops to create more filler opportunities */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target5:
            /* Safe multiplication (by power of 2) - no traps */
            c = d * 2;  /* Shift operation, safe for delay slot */
        }
        
        /* Final updates for next iteration */
        a = (a * 3) / 2;
        b = b + i;
        result = result & 0xFF;  /* Prevent overflow */
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional test case: Function with multiple return paths */
    /* This creates more branch-label patterns for the compiler */
    {
        register int x REG1 = 10;
        register int y REG2 = 20;
        register int z REG3 = 0;
        
        if (__builtin_expect(x < y, 1)) {
            asm volatile("nop" ::: "memory");
            ret_target1:
            z = x + y;  /* Eligible delay slot candidate */
        }
        
        printf("Test value: %d\n", z);
    }
    
    return result != 0;
}

/* Additional function to create more compilation units with branches */
static int helper_func(int n) {
    register int p REG1 = n;
    register int q REG2 = n * 2;
    register int r REG3 = 0;
    
    /* Create predictable branch pattern */
    if (__builtin_expect(p > 50, 0)) {
        asm volatile("nop" ::: "memory");
        helper_target:
        /* Simple, safe operation */
        r = q - p;  /* No memory, no traps, no function calls */
    }
    
    return r;
}
