/* This program is designed to trigger GCC's delay slot filling logic
   specifically targeting the uncovered lines in reorg.cc (lines 2135-2149).
   It creates conditional branches where the instruction after the branch
   label is a safe, non-jump candidate for delay slot filling. */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of loop and branches */
static volatile int iterations = 100;

/* Use register variables to control register allocation and avoid conflicts */
register int a asm("$2");  /* Branch condition variable 1 */
register int b asm("$3");  /* Branch condition variable 2 */
register int c asm("$4");  /* Candidate instruction variable 1 */
register int d asm("$5");  /* Candidate instruction variable 2 */
register int e asm("$6");  /* Loop counter */
register int f asm("$7");  /* Result accumulator */

int main(void) {
    /* Initialize variables - using different registers for branch vs candidate */
    a = 0;
    b = 100;
    c = 1;
    d = 2;
    e = 0;
    f = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (e = 0; e < iterations; e++) {
        /* Vary branch conditions to create different execution paths */
        a = e * 3;
        b = iterations - e;
        
        /* BRANCH 1: Predictable taken branch with nop filler */
        if (__builtin_expect(a < b, 1)) {
            /* This asm creates a nop that the delay slot filler may try to replace */
            asm volatile("nop" ::: "memory");
            /* Target label for the branch */
            target_label_1:
            /* CANDIDATE INSTRUCTION: Simple arithmetic, uses different registers than branch condition */
            c = d + 1;  /* This is the candidate for delay slot filling */
            f += c;
        }
        
        /* Reset variables for next branch */
        c = e;
        d = e + 10;
        
        /* BRANCH 2: Predictable not-taken branch with multiple nops */
        if (__builtin_expect(a > b + 50, 0)) {
            /* Multiple nops to give filler multiple trial instructions */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_2:
            /* Another candidate instruction - register move operation */
            d = c + 5;  /* Safe, trap-free, no resource conflicts */
            f += d;
        }
        
        /* BRANCH 3: Varying condition with different register usage */
        register int x asm("$8") = e * 2;
        register int y asm("$9") = e + 20;
        
        if (__builtin_expect(x != y, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_3:
            /* Candidate uses completely different register set ($10, $11) */
            register int p asm("$10") = f;
            register int q asm("$11") = 7;
            p = q + 3;  /* Simple arithmetic, no memory access, no traps */
            f = p;
        }
        
        /* BRANCH 4: More complex pattern with independent operations */
        register int r1 asm("$12") = e;
        register int r2 asm("$13") = iterations;
        
        if (__builtin_expect((r1 & 0x1) == 0, 1)) {
            /* Single nop - simplest case for delay slot filling */
            asm volatile("nop" ::: "memory");
            target_label_4:
            /* Ideal candidate: uses registers not involved in any branch condition */
            register int t1 asm("$14") = 0;
            register int t2 asm("$15") = 1;
            t1 = t2 + 2;  /* Completely independent operation */
            f += t1;
        }
        
        /* Modify variables to change future branch behavior */
        a += 1;
        b -= 1;
        
        /* Prevent loop unrolling to preserve branch structure */
        asm volatile("" : "+r"(e) : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", f);
    
    /* Additional test case with nested branches */
    {
        register int v1 asm("$16") = 100;
        register int v2 asm("$17") = 200;
        register int v3 asm("$18") = 0;
        register int v4 asm("$19") = 1;
        
        /* Outer branch */
        if (__builtin_expect(v1 < v2, 1)) {
            asm volatile("nop" ::: "memory");
            outer_label:
            /* Candidate for outer branch delay slot */
            v3 = v4 + 10;
            
            /* Inner branch - creates more complex control flow */
            if (__builtin_expect(v3 > 5, 1)) {
                asm volatile("nop" ::: "memory");
                inner_label:
                /* Candidate for inner branch delay slot */
                v4 = v3 - 5;
            }
        }
        
        f += v3 + v4;
    }
    
    return f > 0 ? 0 : 1;
}
