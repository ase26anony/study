/* reorg_delay_slot_test.c
 * Target: Trigger uncovered lines 2135-2149 in reorg.cc
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

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    int result = 0;
    
    /* Initialize register variables */
    a = 1; b = 2; c = 3; d = 4;
    e = 5; f = 6; g = 7; h = 8;
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Conditional with predictable pattern */
        /* Use __builtin_expect to guide branch prediction */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
            /* Target label for delay slot candidate */
            target_label_1:
            /* Simple arithmetic - safe for delay slot */
            /* Uses registers not involved in branch condition */
            e = f + 1;  /* Candidate for delay slot filling */
            /* Continue with other operations */
            c = d * 2;
        } else {
            a = b + i;
        }
        
        /* BRANCH 2: Different condition, different registers */
        /* Create variation in branch patterns */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for more slots */
            target_label_2:
            /* Another simple arithmetic operation */
            g = h - 1;  /* Another delay slot candidate */
            f = e * 3;
        } else {
            d = c - i;
        }
        
        /* BRANCH 3: Nested condition for complex flow */
        if (__builtin_expect(e != f, 0)) {
            /* Multiple nops to create more filler opportunities */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_3:
            /* Simple move operation - very safe for delay slot */
            a = b;  /* Register move as delay slot candidate */
            /* Follow with arithmetic that uses result */
            result += a;
        }
        
        /* BRANCH 4: Complex condition with arithmetic */
        int temp = a + b;
        if (__builtin_expect(temp > c, 1)) {
            /* Varying number of nops */
            asm volatile("nop" ::: "memory");
            target_label_4:
            /* Safe arithmetic with immediate */
            h = g + 2;  /* Immediate add as delay slot candidate */
            result += h;
        }
        
        /* Modify variables to change branch outcomes */
        a = (a * 3 + 1) % 100;
        b = (b * 5 + 2) % 100;
        c = (c * 7 + 3) % 100;
        d = (d * 11 + 4) % 100;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d),
                        "+r"(e), "+r"(f), "+r"(g), "+r"(h));
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", result + a + b + c + d + e + f + g + h);
    
    return 0;
}

/* Additional test function to create more opportunities */
void test_delay_slots(int x, int y) {
    register int r1 asm("$s0");
    register int r2 asm("$s1");
    register int r3 asm("$s2");
    register int r4 asm("$s3");
    
    r1 = x;
    r2 = y;
    
    /* Multiple branches in sequence */
    for (int i = 0; i < 50; i++) {
        /* Branch with simple condition */
        if (__builtin_expect(r1 > r2, 0)) {
            asm volatile("nop" ::: "memory");
            delay_target_1:
            /* Very simple operation - ideal delay slot */
            r3 = r4;  /* Pure register move */
            r1 = r1 - 1;
        }
        
        /* Another branch */
        if (__builtin_expect(r2 < r3, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            delay_target_2:
            /* Arithmetic with small immediate */
            r4 = r3 + 5;  /* Safe immediate addition */
            r2 = r2 + 2;
        }
        
        /* Swap values to change branch behavior */
        int tmp = r1;
        r1 = r2;
        r2 = tmp;
    }
}
