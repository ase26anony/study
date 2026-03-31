/* reorg_delay_slot_test.c
 * Target: GCC delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int a asm("$2");
register int b asm("$3");
register int c asm("$4");
register int d asm("$5");
register int e asm("$6");
register int f asm("$7");
register int g asm("$8");
register int h asm("$9");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Function to create predictable branch patterns */
int process_value(int x) {
    /* Simple arithmetic to create independent operations */
    int y = x * 3;
    int z = y + 7;
    return z - 2;
}

int main(void) {
    int result = 0;
    
    /* Initialize registers with distinct values */
    a = 1; b = 2; c = 3; d = 4;
    e = 5; f = 6; g = 7; h = 8;
    
    /* Main loop - prevents optimization and preserves branch structure */
    for (volatile int i = 0; i < iterations; i++) {
        
        /* ====== BRANCH PATTERN 1: Likely taken branch ====== */
        /* Create a conditional branch with predictable outcome */
        if (__builtin_expect(a < b, 1)) {
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
            /* Target label for the branch */
            target_label_1:
            /* ELIGIBLE DELAY SLOT CANDIDATE:
               Simple arithmetic using registers not involved in branch condition
               (c and d are independent from a and b) */
            c = d + 1;  /* This should be moved into delay slot */
        }
        
        /* ====== BRANCH PATTERN 2: Unlikely taken branch ====== */
        /* Different probability to trigger different code paths */
        if (__builtin_expect(e > f, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slots */
            target_label_2:
            /* Another eligible candidate using different registers */
            g = h - 2;  /* Independent operation */
        }
        
        /* ====== BRANCH PATTERN 3: Variable condition ====== */
        /* Mix of taken/not-taken to keep both paths live */
        int temp = process_value(i);
        if (__builtin_expect((temp & 1) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
            target_label_3:
            /* Multiple independent operations after label */
            a = b + c;  /* Uses previously set registers */
            d = e * 2;  /* Another simple operation */
        }
        
        /* ====== BRANCH PATTERN 4: Nested branches ====== */
        /* Create complex control flow */
        if (__builtin_expect(g != 0, 1)) {
            asm volatile("nop" ::: "memory");
            target_label_4:
            /* Safe operation - no function calls, no memory access */
            f = (g << 2) | 1;
            
            /* Inner branch to create more opportunities */
            if (__builtin_expect(h < 100, 1)) {
                asm volatile("nop" ::: "memory");
                inner_label:
                /* Another candidate */
                e = f ^ 0xFF;
            }
        }
        
        /* ====== BRANCH PATTERN 5: Loop-dependent branch ====== */
        /* Branch condition changes over time */
        if (__builtin_expect(i % 3 == 0, 0)) {
            asm volatile("nop" ::: "memory");
            target_label_5:
            /* Bit manipulation - safe and trap-free */
            h = (h << 1) | (h >> 31);  /* Rotate right by 1 */
        }
        
        /* Update variables to change branch outcomes */
        a = b + 1;
        b = c - 1;
        c = d * 2;
        d = e / 2;  /* Safe division by constant 2 */
        e = f | g;
        f = g & h;
        g = h ^ a;
        h = (h + i) & 0xFF;  /* Keep within bounds */
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f + g + h;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Additional test case with switch statement */
    {
        int x = result % 5;
        switch (x) {
            case 0:
                asm volatile("nop" ::: "memory");
                switch_label_0:
                a = b + c;
                break;
            case 1:
                asm volatile("nop" ::: "memory");
                switch_label_1:
                b = c + d;
                break;
            case 2:
                asm volatile("nop" ::: "memory");
                switch_label_2:
                c = d + e;
                break;
            default:
                asm volatile("nop" ::: "memory");
                switch_label_default:
                d = e + f;
                break;
        }
    }
    
    return result > 0 ? 0 : 1;
}

/* Helper function to create more branch opportunities */
static int helper(int x, int y) {
    /* Multiple return points create branch instructions */
    if (__builtin_expect(x > y, 0)) {
        asm volatile("nop" ::: "memory");
        helper_label_1:
        return x - y;
    } else if (__builtin_expect(x == y, 0)) {
        asm volatile("nop" ::: "memory");
        helper_label_2:
        return x * 2;
    } else {
        asm volatile("nop" ::: "memory");
        helper_label_3:
        return y - x;
    }
}
