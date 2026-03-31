/* Program to trigger GCC delay slot filling logic in reorg.cc lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
register int r0 asm("$t0");
register int r1 asm("$t1");
register int r2 asm("$t2");
register int r3 asm("$t3");
register int r4 asm("$t4");
register int r5 asm("$t5");

int main() {
    volatile int iterations = 100;  /* Prevent loop unrolling */
    int result = 0;
    
    /* Initialize registers with distinct values */
    r0 = 0;
    r1 = 1;
    r2 = 2;
    r3 = 3;
    r4 = 4;
    r5 = 5;
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch conditions to create different execution paths */
        int condition = (i % 3);
        
        /* BRANCH 1: Simple conditional with predictable outcome */
        if (__builtin_expect(r0 < r1, 1)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
            /* Target label with simple arithmetic instruction */
            /* This instruction uses registers NOT involved in branch condition */
            r4 = r5 + 1;  /* Candidate for delay slot filling */
            /* Continue with other operations */
            r0 = r0 + i;
        }
        
        /* BRANCH 2: Different condition with opposite prediction */
        if (__builtin_expect(r2 > r3, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for more slots to fill */
            r0 = r1 + 2;  /* Another candidate using different registers */
            r2 = r2 - 1;
        }
        
        /* BRANCH 3: More complex condition with mixed register usage */
        if (__builtin_expect((r0 & 1) == condition, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
            /* Multiple simple instructions after label */
            r3 = r4 * 2;    /* First candidate - simple multiplication */
            r5 = r2 + r3;   /* Second candidate - addition */
            r1 = r5 - 1;    /* Third candidate - subtraction */
        }
        
        /* BRANCH 4: Nested condition to create deeper control flow */
        if (__builtin_expect(i % 5 == 0, 1)) {
            if (__builtin_expect(r4 != r5, 1)) {
                asm volatile("nop" ::: "memory");
                /* Instruction that doesn't trap and uses independent registers */
                r2 = r3 << 1;  /* Left shift - trap-free operation */
                r4 = r4 + i;
            }
        }
        
        /* BRANCH 5: Loop-dependent condition */
        if (__builtin_expect(i < iterations / 2, 1)) {
            asm volatile("nop" ::: "memory");
            /* Simple move operation between registers */
            int temp = r0;
            r0 = r1;
            r1 = temp;  /* Register swap - simple operations */
            r5 = r5 + 2;
        }
        
        /* Update variables to change future branch outcomes */
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        r1 = (r1 * 1103515245 + 12345) & 0x7fffffff;
        r2 = (r2 + i) & 0xff;
        r3 = (r3 - i) & 0xff;
        
        /* Accumulate result to prevent dead code elimination */
        result += r0 + r1 + r2 + r3 + r4 + r5;
    }
    
    /* Use result to prevent optimization */
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Additional function to create more complex control flow */
void create_branch_opportunities(int n) {
    register int a asm("$t6");
    register int b asm("$t7");
    register int c asm("$t8");
    register int d asm("$t9");
    
    a = 10;
    b = 20;
    c = 30;
    d = 40;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent branches with different conditions */
        if (__builtin_expect(a < b, 1)) {
            asm volatile("nop" ::: "memory");
            /* Target instruction: simple arithmetic with constants */
            c = d + 5;  /* Safe, trap-free operation */
            a = a + 1;
        }
        
        if (__builtin_expect(b > c, 0)) {
            asm volatile("nop" ::: "memory");
            d = a * 3;  /* Multiplication with small constant */
            b = b - 1;
        }
        
        /* Update variables */
        a = (a + i) & 0xff;
        b = (b - i) & 0xff;
        c = (c ^ i) & 0xff;  /* XOR operation - trap-free */
        d = (d | i) & 0xff;  /* OR operation - trap-free */
    }
}
