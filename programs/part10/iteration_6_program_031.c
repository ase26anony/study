/* reorg_delay_slot.c
 * Target: GCC reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish reorg_delay_slot.c -o reorg_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
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

int main(void) {
    int result = 0;
    
    /* Initialize registers with distinct values */
    a = 1; b = 2; c = 3; d = 4;
    e = 5; f = 6; g = 7; h = 8;
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Create predictable branch using __builtin_expect */
        /* Branch condition uses registers a and b only */
        if (__builtin_expect(a > b, 0)) {
            /* This branch should be taken rarely (predictable) */
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
            
            /* TARGET LABEL: Place simple arithmetic that doesn't use a or b */
            /* This instruction should be eligible for delay slot filling */
            target_label_1:
            /* Use registers c and d (distinct from branch condition registers) */
            e = c + d;  /* Simple trap-free arithmetic */
            
            /* Continue with more independent operations */
            f = g + 1;
        } else {
            /* More nops to create different filling scenarios */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
        }
        
        /* Second branch with different register usage */
        if (__builtin_expect(c < d, 1)) {
            /* This branch should be taken often */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            target_label_2:
            /* Another simple arithmetic using different registers */
            g = e + f;  /* Register-to-register, no trap */
            
            h = a + 1;
        }
        
        /* Third branch with immediate constant operation */
        if (__builtin_expect(e != f, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            target_label_3:
            /* Operation with immediate constant (safe, no division) */
            a = b + 3;  /* Simple addition with constant */
        }
        
        /* Update variables to change branch outcomes */
        a = (a * 3 + 1) & 0xFF;  /* Keep values bounded */
        b = (b * 5 + 2) & 0xFF;
        c = (c * 7 + 3) & 0xFF;
        d = (d * 11 + 4) & 0xFF;
        
        /* Mix in some register swaps to create different patterns */
        int tmp = e;
        e = f;
        f = tmp;
        
        /* Accumulate result to keep computations live */
        result += (a + b + c + d + e + f + g + h) & 1;
    }
    
    /* Force use of all variables to prevent optimization */
    printf("Result: %d (a=%d, b=%d, c=%d, d=%d, e=%d, f=%d, g=%d, h=%d)\n",
           result, a, b, c, d, e, f, g, h);
    
    return result & 1;
}

/* Additional test function to create more complex branching patterns */
void branch_patterns(void) {
    register int x asm("$s0");
    register int y asm("$s1");
    register int z asm("$s2");
    register int w asm("$s3");
    
    x = 10; y = 20; z = 30; w = 40;
    
    /* Nested branches to create different control flow */
    for (int i = 0; i < 50; i++) {
        /* Outer branch */
        if (__builtin_expect(x > y, 0)) {
            asm volatile("nop" ::: "memory");
            
            outer_label:
            /* Simple move operation - good delay slot candidate */
            z = w;
            
            /* Inner branch */
            if (__builtin_expect(z < 50, 1)) {
                asm volatile("nop" ::: "memory");
                
                inner_label:
                /* Another simple operation */
                x = y + 2;
            }
        }
        
        /* Update variables */
        x = (x + i) & 0x7F;
        y = (y - i) & 0x7F;
        z = (z ^ i) & 0x7F;
        w = (w * 13 + i) & 0x7F;
    }
}
