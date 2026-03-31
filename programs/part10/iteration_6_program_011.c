/* reorg_coverage.c - Target GCC delay slot filling logic */
#include <stdio.h>
#include <stdlib.h>

/* Force predictable branch behavior */
#define UNLIKELY(x) __builtin_expect((x), 0)
#define LIKELY(x) __builtin_expect((x), 1)

int main(void) {
    /* Use register variables to control allocation */
    register int a asm("$2") = 0;  /* Branch condition variable 1 */
    register int b asm("$3") = 1;  /* Branch condition variable 2 */
    register int c asm("$4") = 0;  /* Delay slot candidate source 1 */
    register int d asm("$5") = 0;  /* Delay slot candidate source 2 */
    register int e asm("$6") = 0;  /* Delay slot candidate result */
    register int f asm("$7") = 0;  /* Another independent variable */
    
    volatile int iterations = 100;  /* Prevent loop unrolling */
    int i;
    
    /* Initialize variables with different values to create varying branch patterns */
    c = 10;
    d = 20;
    f = 30;
    
    /* Main loop with multiple branch patterns */
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Branch with single nop before target */
        if (UNLIKELY(a > b)) {
            asm volatile("nop" ::: "memory");  /* Filler for delay slot */
            goto target1;
        }
        /* Some intermediate computation to separate branches */
        f = f + i;
        
        /* Pattern 2: Branch with two nops (forces multiple trial evaluations) */
        if (LIKELY(b >= a)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target2;
        }
        
        /* Pattern 3: Another branch variant */
        if (UNLIKELY((a + i) % 3 == 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target3;
        }
        
        /* Continue normal execution if no branch taken */
        a = a + 1;
        b = b - 1;
        continue;
        
    target1:
        /* Simple arithmetic - eligible for delay slot */
        /* Uses registers not involved in branch condition (c,d vs a,b) */
        e = c + d;  /* This should be moved into delay slot */
        /* Follow-up computation to prevent dead code elimination */
        f = e + f;
        a = a + 2;
        b = b + 1;
        continue;
        
    target2:
        /* Another simple arithmetic operation */
        e = d - c;  /* Also eligible */
        f = f * 2;
        a = a - 1;
        b = b + 2;
        continue;
        
    target3:
        /* Third candidate operation */
        e = c * 2;  /* Multiplication is safe (no trap for ints) */
        f = f / 3;
        a = a + 3;
        b = b - 2;
        continue;
    }
    
    /* Use results to prevent optimization */
    printf("Result: a=%d, b=%d, e=%d, f=%d\n", a, b, e, f);
    
    /* Additional test case with nested branches */
    {
        register int x asm("$8") = 0;
        register int y asm("$9") = 100;
        register int z asm("$10") = 0;
        
        while (x < y) {
            /* Conditional branch with predictable pattern */
            if (UNLIKELY(x % 7 == 0)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                goto compute_label;
            }
            x++;
            continue;
            
        compute_label:
            /* Simple register operation - perfect delay slot candidate */
            z = y - x;  /* No memory access, no function call, no trap */
            x = x + z;
        }
        
        printf("Nested result: x=%d, z=%d\n", x, z);
    }
    
    return e + f;  /* Return value based on computations */
}
