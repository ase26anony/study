/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int a asm("$t0");
register int b asm("$t1");
register int c asm("$t2");
register int d asm("$t3");
register int e asm("$t4");
register int f asm("$t5");
register int g asm("$t6");
register int h asm("$t7");

/* Volatile to prevent optimization */
volatile int iterations = 100;
volatile int seed = 42;

/* Simple trap-free arithmetic operations */
static inline int safe_add(int x, int y) {
    return x + y;  /* No overflow check - compiler knows this won't trap */
}

static inline int safe_mul(int x, int y) {
    return x * y;  /* Using small values to avoid overflow traps */
}

int main(void) {
    int result = 0;
    
    /* Initialize registers with safe values */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    
    /* Loop to create multiple branch opportunities */
    for (volatile int i = 0; i < iterations; i++) {
        /* Varying branch conditions to create different paths */
        int condition = (i & 1) ? (a > b) : (c < d);
        
        /* BRANCH 1: Simple conditional with predictable outcome */
        if (__builtin_expect(condition, 0)) {
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
            /* Target label L1 - simple arithmetic that doesn't conflict */
            L1:
            /* Candidate for delay slot: uses registers not in branch condition */
            e = f + 1;  /* Simple, trap-free, no resource conflict */
            /* Continue with other operations */
            g = safe_add(h, 2);
        } else {
            /* Alternative path */
            f = safe_mul(g, 2);
        }
        
        /* BRANCH 2: Different register set */
        if (__builtin_expect((e & 1) == 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slots */
            L2:
            /* Another candidate: different registers */
            a = b + c;  /* Still simple arithmetic */
            d = safe_add(e, f);
        }
        
        /* BRANCH 3: Nested to create complex flow */
        if (__builtin_expect(g > h, 0)) {
            /* Multiple nops to fill */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            L3:
            /* Candidate using only fresh registers */
            int temp1 = safe_add(a, b);
            int temp2 = safe_mul(c, 2);
            result += temp1 + temp2;
        }
        
        /* BRANCH 4: Forward jump over multiple instructions */
        if (__builtin_expect((i % 3) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            goto L4;
        }
        
        /* Some intermediate code that shouldn't be moved */
        a = safe_add(a, 1);
        b = safe_add(b, seed & 0xF);  /* Small constant to avoid traps */
        
        L4:
        /* Target for forward jump - simple operation */
        c = d + e;  /* Eligible delay slot candidate */
        
        /* BRANCH 5: Backward branch in mini-loop */
        int j = 3;
        do {
            if (__builtin_expect(j > 0, 1)) {
                asm volatile("nop" ::: "memory");
                L5:
                f = g + h;  /* Candidate */
                h = safe_add(h, 1);
            }
            j--;
        } while (j > 0);
        
        /* Modify variables to change branch behavior */
        a ^= (i << 1);
        b += (seed & 0x7);
        c = safe_add(c, 1);
        d = safe_mul(d, (seed & 0x3) + 1);  /* Multiply by 1-4, safe */
        
        /* Ensure no division or memory ops that could trap */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use results to prevent dead code elimination */
    result += a + b + c + d + e + f + g + h;
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Helper function to create additional branching contexts */
static void branch_helper(int x, int y) {
    register int r1 asm("$s0") = x;
    register int r2 asm("$s1") = y;
    register int r3 asm("$s2") = 0;
    
    /* Create another branch opportunity */
    if (__builtin_expect(r1 > r2, 0)) {
        asm volatile("nop" ::: "memory");
        H1:
        /* Simple move/arithmetic for delay slot */
        r3 = r1 + 1;  /* Safe, no traps */
    }
    
    /* Force the helper to be used */
    asm volatile("" : "+r"(r3) : : "memory");
}

/* Additional test case with switch for variety */
static int switch_test(int val) {
    switch (val & 3) {
        case 0:
            asm volatile("nop" ::: "memory");
            S1:
            return val + 1;  /* Simple return expression */
        case 1:
            asm volatile("nop" ::: "memory");
            S2:
            return val * 2;  /* Still trap-free with small values */
        default:
            return val;
    }
}
