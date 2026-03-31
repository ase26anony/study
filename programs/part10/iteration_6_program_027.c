/* reorg_delay_slot_test.c
 * Target: GCC delay slot filler (reorg.cc lines 2135-2149)
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

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Simple trap-free arithmetic operations for delay slot candidates */
#define SAFE_OP1(x, y) ((x) = (y) + 1)      /* No trap possible */
#define SAFE_OP2(x, y, z) ((x) = (y) + (z)) /* Register-to-register add */
#define SAFE_OP3(x, y) ((x) = (y) << 2)     /* Shift by constant */

int main(void) {
    int result = 0;
    
    /* Initialize register variables */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6;
    
    /* 
     * Main loop with multiple branches to exercise delay slot filler.
     * The volatile iterations prevents complete unrolling.
     */
    for (int i = 0; i < iterations; i++) {
        /* ===== BRANCH PATTERN 1: Predictable taken branch ===== */
        /* Create a simple conditional branch that often jumps */
        if (__builtin_expect(a < b, 1)) {
            /* Insert nop that delay slot filler might replace */
            asm volatile("nop" ::: "memory");
            /* Target label for the branch */
            target_label_1:
            /* Candidate for delay slot: simple, trap-free, uses different registers */
            SAFE_OP1(e, f);  /* e = f + 1 */
        }
        
        /* ===== BRANCH PATTERN 2: Predictable not-taken branch ===== */
        /* Different condition, different registers for branch computation */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple trial attempts */
            target_label_2:
            /* Another candidate: register-to-register operation */
            SAFE_OP2(f, a, b);  /* f = a + b */
        }
        
        /* ===== BRANCH PATTERN 3: Varying condition ===== */
        /* Mix of taken/not-taken to keep both paths live */
        if (__builtin_expect((i & 1) == 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
            target_label_3:
            /* Third candidate: shift operation */
            SAFE_OP3(d, c);  /* d = c << 2 */
        }
        
        /* ===== BRANCH PATTERN 4: Complex resource separation ===== */
        /* Use completely separate register set for this branch */
        register int r1 asm("$t6") = i;
        register int r2 asm("$t7") = i * 2;
        register int r3 asm("$t8") = 0;
        register int r4 asm("$t9") = 0;
        
        if (__builtin_expect(r1 != r2, 1)) {
            asm volatile("nop" ::: "memory");
            target_label_4:
            /* Candidate uses t8/t9, branch uses t6/t7 - no resource conflict */
            r3 = r4 + 7;  /* Simple immediate add */
        }
        
        /* Update variables to change branch behavior over time */
        a += i;
        b += i * 2;
        c ^= i;
        d = (d + 1) & 0xFF;
        
        /* Use results to prevent dead code elimination */
        result += e + f;
    }
    
    /* Additional test case: nested branches */
    {
        register int x asm("$s0") = 100;
        register int y asm("$s1") = 200;
        register int z asm("$s2") = 0;
        register int w asm("$s3") = 0;
        
        for (int j = 0; j < 50; j++) {
            /* Outer branch */
            if (__builtin_expect(x > y, 0)) {
                asm volatile("nop" ::: "memory");
                outer_label:
                /* Simple candidate for outer branch delay slot */
                z = w + 3;
                
                /* Inner branch - creates more complex control flow */
                if (__builtin_expect((j & 3) == 0, 1)) {
                    asm volatile("nop" ::: "memory");
                    inner_label:
                    /* Candidate for inner branch delay slot */
                    w = z - 1;
                }
            }
            
            x -= j;
            y += (j & 1);
            result += z + w;
        }
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* 
 * Additional function to create more compilation units with delay slots
 * This increases chances of hitting the uncovered code path
 */
int helper_function(int base) {
    register int p asm("$a0") = base;
    register int q asm("$a1") = base * 2;
    register int r asm("$a2") = 0;
    int sum = 0;
    
    for (int k = 0; k < 25; k++) {
        /* Branch with predictable pattern */
        if (__builtin_expect(p < q, 1)) {
            asm volatile("nop" ::: "memory");
            helper_label:
            /* Delay slot candidate using a2/a3 registers */
            r = (p & 0xF) + 5;  /* Mask ensures no overflow issues */
        }
        
        p += k;
        q -= (k & 1);
        sum += r;
    }
    
    return sum;
}
