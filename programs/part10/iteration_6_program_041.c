/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables to avoid resource conflicts */
register int a asm("$2");  /* Branch condition variable 1 */
register int b asm("$3");  /* Branch condition variable 2 */
register int c asm("$4");  /* Delay slot candidate source 1 */
register int d asm("$5");  /* Delay slot candidate source 2 */
register int e asm("$6");  /* Delay slot candidate destination */
register int f asm("$7");  /* Another independent variable */
register int g asm("$8");  /* Yet another independent variable */

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = 1; b = 2; c = 3; d = 4; e = 0; f = 5; g = 6;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* VARYING NUMBER OF NOPS BEFORE BRANCH - creates different trial scenarios */
        asm volatile("nop" ::: "memory");
        if (i % 3 == 0) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
        }
        
        /* BRANCH 1: Predictable taken branch with __builtin_expect */
        /* This creates a simplejump_p(trial) that jump_to_label_p */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop filler that could be replaced */
            asm volatile("nop" ::: "memory");
            goto target_label_1;
        }
        
        /* Independent computation between branch and label */
        f = g + i;
        
        /* More nops to vary the distance */
        if (i % 2 == 0) {
            asm volatile("nop" ::: "memory");
        }
        
        /* This is the instruction that should be moved into delay slot */
        /* It's after the label, non-jump, non-sequence, no resource conflicts */
        target_label_1:
        /* SIMPLE ARITHMETIC - eligible for delay slot */
        /* Uses registers not involved in branch condition (c,d,e vs a,b) */
        e = c + d;  /* This should be the next_trial candidate */
        
        /* Continue with loop logic */
        result += e;
        
        /* BRANCH 2: Different condition to create another scenario */
        /* Change variables to alter branch prediction */
        a = i;
        b = iterations - i;
        
        if (__builtin_expect(b > a, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label_2;
        }
        
        /* More independent operations */
        g = f * 2;
        
        target_label_2:
        /* Another delay slot candidate - different operation */
        e = d - c;  /* Simple subtraction, no traps */
        
        result += e;
        
        /* BRANCH 3: Nested condition for complex flow */
        c = (i * 3) % 7;
        d = (i * 5) % 11;
        
        if (__builtin_expect(c != d, 0)) {
            /* Multiple nops to create filler slots */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label_3;
        }
        
        f = e + 1;
        
        target_label_3:
        /* Simple move operation - very safe for delay slot */
        g = f;  /* Register move, no computation */
        
        result += g;
        
        /* Update variables to change branch behavior */
        a = (a + 1) % 10;
        b = (b + 2) % 10;
        c = (c + 3) % 10;
        d = (d + 4) % 10;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
