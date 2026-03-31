/* reorg_trigger.c - Target GCC delay slot filling logic */
#include <stdio.h>
#include <stdlib.h>

/* Force predictable branch behavior */
#define UNLIKELY(x) __builtin_expect((x), 0)
#define LIKELY(x) __builtin_expect((x), 1)

int main(void) {
    /* Use register variables to control allocation */
    register int a asm("$2") = 0;  /* MIPS $2 (v0) - branch condition var 1 */
    register int b asm("$3") = 1;  /* MIPS $3 (v1) - branch condition var 2 */
    register int c asm("$4") = 0;  /* MIPS $4 (a0) - delay slot candidate src 1 */
    register int d asm("$5") = 0;  /* MIPS $5 (a1) - delay slot candidate src 2 */
    register int e asm("$6") = 0;  /* MIPS $6 (a2) - delay slot candidate dest */
    register int f asm("$7") = 0;  /* MIPS $7 (a3) - temp for operations */
    
    volatile int iterations = 100;  /* Prevent loop unrolling */
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* 
         * BRANCH 1: Conditional jump with predictable outcome
         * This creates a simplejump_p(trial) that jump_to_label_p
         */
        if (UNLIKELY(a > b)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
            /* TARGET LABEL 1 - next_trial candidate follows */
target1:
            /* 
             * Eligible delay slot candidate:
             * - Simple arithmetic (no trap, no memory access)
             * - Uses registers NOT used in branch condition (avoid resource conflicts)
             * - Non-jump, non-sequence instruction
             */
            e = c + d;  /* $6 = $4 + $5 */
            
            /* Continue execution */
            f = e * 2;
        }
        
        /* 
         * BRANCH 2: Different condition to create multiple trial opportunities
         * slots_to_fill != slots_filled triggers re-evaluation
         */
        asm volatile("nop" ::: "memory");  /* Extra filler */
        asm volatile("nop" ::: "memory");  /* Multiple nops create more slots */
        
        if (LIKELY(b != 0)) {
            asm volatile("nop" ::: "memory");
target2:
            /* Another eligible candidate - register move operation */
            f = c;  /* $7 = $4 - simple move */
            
            /* Independent operation to avoid conflicts */
            a = i + 1;
        }
        
        /* 
         * BRANCH 3: Complex condition to force multiple delay slot attempts
         * The filler will try different trial instructions
         */
        if (UNLIKELY((a & 1) == 0)) {
            /* Multiple nops to create multiple filler slots */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target3:
            /* Safe arithmetic - immediate constant (no trap) */
            d = c + 3;  /* $5 = $4 + 3 */
            
            /* Follow with another simple operation */
            e = d - 1;
        }
        
        /* 
         * BRANCH 4: Nested condition to create complex control flow
         */
        if (a < iterations) {
            asm volatile("nop" ::: "memory");
            if (UNLIKELY(b > a)) {
                asm volatile("nop" ::: "memory");
target4:
                /* Bitwise operation - trap-free */
                f = c & 0xFF;  /* $7 = $4 & 0xFF */
                
                /* Update branch condition variables */
                b = a + f;
            }
        }
        
        /* Modify variables to change branch outcomes */
        a = (a + 1) % 50;
        b = (b + 2) % 50;
        c = (c + i) % 100;
        d = (d + 3) % 100;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e), "+r"(f));
    }
    
    /* Use results to prevent optimization */
    printf("Result: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d\n", a, b, c, d, e, f);
    
    return (a + b + c + d + e + f) > 0 ? 0 : 1;
}

/* 
 * Additional function to create more complex control flow graph
 * This helps create more opportunities for delay slot filling
 */
void helper_function(int *arr, int n) {
    register int x asm("$8") = 0;
    register int y asm("$9") = 1;
    register int z asm("$10") = 0;
    
    for (int i = 0; i < n; i++) {
        /* Another branch with delay slot candidate */
        if (UNLIKELY(arr[i] > 0)) {
            asm volatile("nop" ::: "memory");
helper_target:
            /* Safe arithmetic for delay slot */
            z = x + y;  /* $10 = $8 + $9 */
            
            arr[i] = z;
        }
        
        x = y;
        y = z;
        z = i;
    }
}
