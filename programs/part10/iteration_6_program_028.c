/* delay_slot_filler.c
 * Target: GCC reorg.cc lines 2135-2149
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
    
    /* Initialize registers with non-zero values */
    a = 1; b = 2; c = 3; d = 4;
    e = 5; f = 6; g = 7; h = 8;
    
    /* Main loop with multiple branch patterns */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Branch with single nop before target */
        if (__builtin_expect(a > b, 0)) {
            /* Force compiler to generate a conditional branch */
            asm volatile("nop" ::: "memory");
            goto target_label_1;
        }
        
        /* Filler nop that might be replaced */
        asm volatile("nop" ::: "memory");
        
        /* Branch target label with simple arithmetic */
        target_label_1:
        /* Simple trap-free arithmetic using different registers */
        e = f + 1;  /* Candidate for delay slot filling */
        
        /* Update variables to change branch behavior */
        a = (a * 3 + i) & 0xFF;
        b = (b * 5 + i) & 0xFF;
        
        /* Pattern 2: Different branch with multiple nops */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label_2;
        }
        
        /* Multiple nops creating different slot filling scenarios */
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
        target_label_2:
        /* Another simple arithmetic candidate */
        g = h + 2;  /* Uses different register set */
        
        /* Pattern 3: Nested branches */
        if (__builtin_expect(e != f, 0)) {
            if (__builtin_expect(g > h, 1)) {
                asm volatile("nop" ::: "memory");
                goto target_label_3;
            }
        }
        
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
        target_label_3:
        /* Register move operation - very simple candidate */
        c = d;  /* Simple move, no traps, no complex resources */
        
        /* Update all variables to create varying patterns */
        c = (c + i) & 0xFF;
        d = (d * 2) & 0xFF;
        e = (e - 1) & 0xFF;
        f = (f + i) & 0xFF;
        g = (g ^ h) & 0xFF;
        h = (h + 7) & 0xFF;
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f + g + h;
    }
    
    /* Pattern 4: Final branch outside loop */
    if (__builtin_expect(result > 1000, 1)) {
        asm volatile("nop" ::: "memory");
        goto final_target;
    }
    
    asm volatile("nop" ::: "memory");
    
    final_target:
    /* Final delay slot candidate */
    a = b + c;  /* Simple arithmetic with no side effects */
    
    printf("Result: %d\n", result + a);
    return 0;
}
