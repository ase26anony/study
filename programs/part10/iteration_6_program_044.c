/* reorg_delay_slot_test.c
 * Target: Trigger GCC's delay slot filler logic in reorg.cc lines 2135-2149
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * Or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2 (for SPARC)
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
#ifdef __mips__
register int a asm("$2");
register int b asm("$3");
register int c asm("$4");
register int d asm("$5");
register int e asm("$6");
register int f asm("$7");
register int g asm("$8");
register int h asm("$9");
#elif __sparc__
register int a asm("%l0");
register int b asm("%l1");
register int c asm("%l2");
register int d asm("%l3");
register int e asm("%l4");
register int f asm("%l5");
register int g asm("%l6");
register int h asm("%l7");
#else
/* Generic register hints - compiler will choose appropriate registers */
register int a;
register int b;
register int c;
register int d;
register int e;
register int f;
register int g;
register int h;
#endif

/* Function to create simple, trap-free arithmetic operations */
static inline int safe_add(int x, int y) {
    return x + y;  /* No overflow check - simple addition won't trap */
}

static inline int safe_mul(int x, int y) {
    return x * y;  /* Multiplication with small values won't trap */
}

int main(void) {
    volatile int iterations = 100;  /* Prevent loop unrolling */
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    
    /* Main loop to force multiple delay slot filling attempts */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Conditional branch with predictable outcome */
        /* Use __builtin_expect to guide branch prediction */
        if (__builtin_expect((a > b), 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
target_label_1:
            /* Candidate for delay slot: simple arithmetic with registers
               not used in branch condition (a,b) */
            c = d + 1;  /* Uses c,d - independent of a,b */
        } else {
            /* Alternative path */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
        }
        
        /* Pattern 2: Another branch with different register usage */
        if (__builtin_expect((c < d), 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_2:
            /* Another candidate: different registers */
            e = f * 2;  /* Simple multiplication - won't trap with these values */
        }
        
        /* Pattern 3: More complex condition with multiple nops */
        if (__builtin_expect((e != f) && (g < h), 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_3:
            /* Candidate using safe inline function */
            a = safe_add(b, 3);  /* b is from different set than e,f,g,h */
        }
        
        /* Pattern 4: Nested conditions to create more opportunities */
        if (__builtin_expect((a % 2) == 0, 1)) {
            if (__builtin_expect(b > 0, 1)) {
                asm volatile("nop" ::: "memory");
target_label_4:
                /* Simple move operation - ideal delay slot candidate */
                g = h;
            }
        }
        
        /* Update variables to change branch patterns */
        a = (a * 3 + 1) & 0xFF;  /* Keep values small to avoid overflow */
        b = (b + i) & 0xFF;
        c = (c ^ d) & 0xFF;
        d = (d - 1) & 0xFF;
        e = (e + 2) & 0xFF;
        f = (f * 3) & 0xFF;
        g = (g + h) & 0xFF;
        h = (h + 1) & 0xFF;
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f + g + h;
    }
    
    /* Additional test cases outside loop */
    
    /* Test case 5: Direct jump to label with simple instruction after */
    {
        int x = 10, y = 20, z = 0;
        if (__builtin_expect(x < y, 1)) {
            asm volatile("nop" ::: "memory");
target_label_5:
            z = x + 5;  /* Simple arithmetic after label */
        }
        result += z;
    }
    
    /* Test case 6: Multiple independent operations after label */
    {
        int p = 1, q = 2, r = 3, s = 4;
        if (__builtin_expect(p != 0, 1)) {
            asm volatile("nop" ::: "memory");
target_label_6:
            /* Multiple simple operations - compiler might try to move
               one into delay slot */
            r = q + 1;
            s = p * 2;
        }
        result += r + s;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Helper function to ensure certain patterns aren't optimized away */
__attribute__((noinline))
static void use_values(int val1, int val2) {
    volatile int sink __attribute__((unused));
    sink = val1;
    sink = val2;
}

/* Additional test function with more complex control flow */
void test_delay_slots(void) {
    register int r1 asm("$10") = 100;
    register int r2 asm("$11") = 200;
    register int r3 asm("$12") = 300;
    register int r4 asm("$13") = 400;
    
    volatile int count = 50;
    
    while (count--) {
        /* Branch with varying numbers of nops before target */
        switch (count % 4) {
            case 0:
                if (__builtin_expect(r1 < r2, 1)) {
                    asm volatile("nop" ::: "memory");
case0_label:
                    r3 = r4 + count;  /* Candidate instruction */
                }
                break;
            case 1:
                if (__builtin_expect(r2 > r3, 0)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
case1_label:
                    r1 = r2 - 1;  /* Another candidate */
                }
                break;
            case 2:
                if (__builtin_expect(r3 != r4, 1)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
case2_label:
                    r4 = r1 * 2;  /* Simple multiplication */
                }
                break;
            case 3:
                if (__builtin_expect((r1 + r2) > r3, 0)) {
                    asm volatile("nop" ::: "memory");
case3_label:
                    r2 = r3;  /* Simple register move */
                }
                break;
        }
        
        /* Update registers */
        r1 = (r1 + 1) & 0x7F;
        r2 = (r2 - 1) & 0x7F;
        r3 = (r3 ^ r4) & 0x7F;
        r4 = (r4 + 2) & 0x7F;
        
        use_values(r1, r2);
    }
}
