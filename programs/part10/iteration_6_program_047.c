/* reorg_coverage.c - Target GCC's delay slot filler for uncovered lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

/* Force the compiler to keep branches as-is */
static volatile int global_counter = 0;

int main(void) {
    /* Use register variables to control allocation */
    register int a asm("$2") = 0;  /* Branch condition variable 1 */
    register int b asm("$3") = 1;  /* Branch condition variable 2 */
    register int c asm("$4") = 0;  /* Delay slot candidate source 1 */
    register int d asm("$5") = 10; /* Delay slot candidate source 2 */
    register int e asm("$6") = 0;  /* Delay slot result */
    register int f asm("$7") = 0;  /* Another independent variable */
    
    volatile int iterations = 100;
    int i;
    
    /* Loop to create multiple branch filling opportunities */
    for (i = 0; i < iterations; i++) {
        /* Vary branch outcomes to prevent optimization */
        global_counter++;
        
        /* BRANCH 1: Single nop between branch and label */
        /* This creates a trial instruction for the filler */
        if (__builtin_expect(a > b, 0)) {
            asm volatile("nop" ::: "memory");
            goto target_label1;
        }
        
        /* Some intermediate computation to separate branches */
        f = a + b + i;
        
        /* BRANCH 2: Two nops, testing slots_filled vs slots_to_fill */
        if (__builtin_expect(b < a + i, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label2;
        }
        
        /* More intermediate code */
        a = (a + 1) & 0x7F;  /* Keep a small to avoid overflow issues */
        
        /* BRANCH 3: Three nops, maximum typical delay slots */
        if (__builtin_expect((a ^ b) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label3;
        }
        
        /* Skip to avoid executing delay slot candidates directly */
        goto skip_labels;
        
    target_label1:
        /* DELAY SLOT CANDIDATE 1: Simple arithmetic, no traps */
        /* Uses registers not involved in branch condition (c, d, e) */
        /* This should be eligible for moving into delay slot */
        e = c + d;
        goto after_label;
        
    target_label2:
        /* DELAY SLOT CANDIDATE 2: Different simple operation */
        /* Independent of branch resources */
        e = d - c;
        goto after_label;
        
    target_label3:
        /* DELAY SLOT CANDIDATE 3: Immediate operation */
        e = c + 1;  /* Safe immediate, no trap possible */
        /* Fall through */
        
    after_label:
        /* Use result to keep computation live */
        f += e;
        
    skip_labels:
        /* Update variables to change branch behavior */
        b = (b * 3 + 1) & 0x7F;
        c = (c + 2) & 0xFF;
        d = (d - 1) & 0xFF;
        
        /* Prevent loop unrolling */
        if (global_counter & 1) {
            a = i;
        }
    }
    
    /* Additional test case with nested branches */
    {
        register int x asm("$8") = 0;
        register int y asm("$9") = 100;
        register int z asm("$10") = 0;
        
        /* Complex pattern with multiple labels */
        for (int j = 0; j < 50; j++) {
            x = j * 2;
            y = 100 - j;
            
            /* Conditional with predictable pattern */
            if (__builtin_expect(x < y, 1)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                goto complex_target;
            }
            
            z = x * y;
            continue;
            
        complex_target:
            /* Another eligible delay slot candidate */
            /* Register move/arithmetic with no side effects */
            z = x + 5;  /* Safe, trap-free operation */
            
            /* Force another branch evaluation */
            if (z > 50) {
                asm volatile("nop" ::: "memory");
                goto secondary_target;
            }
            
            y = z;
            continue;
            
        secondary_target:
            /* Final simple operation */
            y = z - 3;
        }
        
        /* Use variables to prevent dead code elimination */
        printf("Result: %d\n", z + y);
    }
    
    /* Final output to keep program useful */
    printf("Final values: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d, global=%d\n",
           a, b, c, d, e, f, global_counter);
    
    return (a + b + c + d + e + f) & 0xFF;
}
