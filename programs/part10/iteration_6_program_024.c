/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
register int a asm("$2");  /* Branch condition variable 1 */
register int b asm("$3");  /* Branch condition variable 2 */
register int c asm("$4");  /* Delay slot candidate variable 1 */
register int d asm("$5");  /* Delay slot candidate variable 2 */
register int e asm("$6");  /* Delay slot candidate variable 3 */
register int f asm("$7");  /* Delay slot candidate variable 4 */

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Simple trap-free arithmetic operations for delay slot candidates */
#define SAFE_OP1(x, y) ((x) + (y))        /* Addition - cannot trap */
#define SAFE_OP2(x, y) ((x) - (y))        /* Subtraction - cannot trap */
#define SAFE_OP3(x)    ((x) + 1)          /* Increment - cannot trap */
#define SAFE_OP4(x)    ((x) - 1)          /* Decrement - cannot trap */

int main(void) {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = 1;
    b = 2;
    c = 3;
    d = 4;
    e = 5;
    f = 6;
    
    /* Loop to create multiple branch opportunities */
    for (volatile int i = 0; i < iterations; i++) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop to create filler instruction */
            asm volatile("nop" ::: "memory");
            /* Target label for branch - instruction here should be eligible */
target1:
            /* Simple, trap-free, non-jump instruction using different registers */
            e = SAFE_OP1(c, d);  /* c + d -> e, uses registers not in branch condition */
        }
        
        /* Update variables to change branch behavior */
        a = b + i;
        b = a - 1;
        
        /* BRANCH 2: Different condition with multiple nops */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target2:
            /* Another simple trap-free instruction */
            f = SAFE_OP2(e, 2);  /* e - 2 -> f */
        }
        
        /* Update more variables */
        c = d + i;
        d = c * 2;
        
        /* BRANCH 3: Complex condition with single nop */
        if (__builtin_expect((a & 1) == 0, 0)) {
            asm volatile("nop" ::: "memory");
target3:
            /* Simple increment operation */
            c = SAFE_OP3(c);  /* c + 1 -> c */
        }
        
        /* BRANCH 4: Nested control flow to create more opportunities */
        if (__builtin_expect(e > 10, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target4:
            /* Simple decrement operation */
            d = SAFE_OP4(d);  /* d - 1 -> d */
            
            /* Additional independent operation to create more candidates */
            f = e + d;
        }
        
        /* BRANCH 5: Switch-like pattern */
        switch (i % 4) {
            case 0:
                if (__builtin_expect(f > 20, 0)) {
                    asm volatile("nop" ::: "memory");
target5:
                    a = b + c;  /* Simple addition */
                }
                break;
            case 1:
                if (__builtin_expect(a < 5, 1)) {
                    asm volatile("nop" ::: "memory");
target6:
                    b = c - d;  /* Simple subtraction */
                }
                break;
        }
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f;
        
        /* Prevent aggressive optimization */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), 
                          "+r"(d), "+r"(e), "+r"(f));
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
