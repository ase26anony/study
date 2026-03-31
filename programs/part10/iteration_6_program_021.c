/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables to avoid resource conflicts */
register int a asm("$2");  /* MIPS $v0 / SPARC %o0 - branch condition var 1 */
register int b asm("$3");  /* MIPS $v1 / SPARC %o1 - branch condition var 2 */
register int c asm("$4");  /* MIPS $a0 / SPARC %o2 - delay slot candidate source */
register int d asm("$5");  /* MIPS $a1 / SPARC %o3 - delay slot candidate dest */
register int e asm("$6");  /* MIPS $a2 / SPARC %o4 - independent computation */
register int f asm("$7");  /* MIPS $a3 / SPARC %o5 - independent computation */

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = 1;
    b = 2;
    c = 3;
    d = 4;
    e = 5;
    f = 6;
    
    /* Main loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* VARYING BRANCH PATTERN 1: Predictable taken branch */
        /* This creates a simplejump_p(trial) that jumps_to_label_p */
        if (__builtin_expect(a < b, 1)) {
            /* Insert nop filler that delay slot filler will try to replace */
            asm volatile("nop" ::: "memory");
            /* Target label for the branch */
            target_label_1:
            /* ELIGIBLE DELAY SLOT CANDIDATE: Simple arithmetic, no traps,
               uses different registers than branch condition (c/d vs a/b) */
            d = c + 1;  /* This should be movable into delay slot */
            /* Continue with other operations */
            e = f * 2;
        }
        
        /* Update variables to change branch behavior */
        a = b + i;
        b = a - 1;
        
        /* VARYING BRANCH PATTERN 2: Predictable not-taken branch */
        /* Different branch pattern to explore different paths in delay slot filler */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for slots_to_fill != slots_filled */
            target_label_2:
            /* Another eligible candidate: register move operation */
            f = e;  /* Simple move, no resource conflicts */
            /* Independent operation to keep program live */
            result += f;
        }
        
        /* VARYING BRANCH PATTERN 3: Unpredictable branch */
        /* Force evaluation of multiple trial instructions */
        if (__builtin_expect((a ^ b) & 1, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_3:
            /* Safe arithmetic with immediate constant (no trap) */
            c = d + 5;  /* add with small constant - trap-free */
            /* Ensure no sequence formation by keeping it simple */
            result ^= c;
        }
        
        /* Update variables for next iteration */
        c = d + i;
        d = c * 2;
        e = f - i;
        f = e + 3;
        
        /* Prevent dead code elimination */
        result += a + b + c + d;
    }
    
    /* VARYING BRANCH PATTERN 4: Nested branches */
    /* Create more complex control flow for delay slot analysis */
    if (__builtin_expect(result > 1000, 1)) {
        asm volatile("nop" ::: "memory");
        target_label_4:
        /* Multiple independent operations after label */
        a = b + c;  /* First operation - delay slot candidate */
        d = a * 2;  /* Second operation - stays after delay slot */
        result += d;
    }
    
    /* Final computation using all variables to keep them live */
    int final_result = a + b + c + d + e + f + result;
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Additional function to create more code context */
void helper_function(int x) {
    /* Create another branch context with different register usage */
    register int r1 asm("$8");
    register int r2 asm("$9");
    register int r3 asm("$10");
    
    r1 = x;
    r2 = x * 2;
    
    if (__builtin_expect(r1 != r2, 1)) {
        asm volatile("nop" ::: "memory");
        helper_label:
        /* Another delay slot candidate in different function context */
        r3 = r1 + r2;  /* Simple addition, no conflicts */
        /* Use result to prevent elimination */
        printf("Helper: %d\n", r3);
    }
}
