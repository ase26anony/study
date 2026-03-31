/* delay_slot_filler.c
 * Targets GCC's delay slot filling logic in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish delay_slot_filler.c -o delay_slot_filler
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
#define REGISTER_VAR(name, reg) register int name asm(reg)

/* Simple trap-free arithmetic operations for delay slot candidates */
static inline int safe_add(int a, int b) {
    return a + b;  /* No trap possible for integer addition */
}

static inline int safe_mul(int a, int b) {
    return a * b;  /* Multiplication won't trap with these values */
}

int main(void) {
    /* Use register variables to control resource allocation */
    REGISTER_VAR(a, "$t0") = 0;    /* Branch condition variable 1 */
    REGISTER_VAR(b, "$t1") = 100;  /* Branch condition variable 2 */
    REGISTER_VAR(c, "$t2") = 0;    /* Delay slot candidate variable 1 */
    REGISTER_VAR(d, "$t3") = 1;    /* Delay slot candidate variable 2 */
    REGISTER_VAR(e, "$t4") = 2;    /* Additional variable for operations */
    REGISTER_VAR(f, "$t5") = 3;    /* Additional variable for operations */
    
    volatile int iterations = 100;  /* Prevent loop unrolling */
    volatile int seed = 42;         /* Force variable values to be live */
    int result = 0;
    
    /* Create multiple branches with different patterns to trigger
       delay slot filling attempts */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Branch with predictable outcome (cold path) */
        if (__builtin_expect(a > b, 0)) {
            /* This branch is unlikely - creates opportunity for delay slot filling */
            asm volatile("nop" :::);  /* Filler instruction for compiler to replace */
target_label_1:
            /* This is the candidate instruction for delay slot filling */
            /* Uses different registers than the branch condition (a,b) */
            c = safe_add(d, e);  /* Simple, trap-free operation */
            
            /* Additional independent operations to create more candidates */
            f = safe_mul(c, 2);
        }
        
        /* Pattern 2: Branch with different register usage */
        if (__builtin_expect(c < f, 1)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);  /* Two nops - multiple filler slots */
target_label_2:
            /* Another candidate using different registers */
            e = safe_add(f, 1);
            
            /* Create a small basic block after label */
            d = safe_mul(e, 3);
        }
        
        /* Pattern 3: Nested branches to create complex control flow */
        if (__builtin_expect((a + i) > (b - i), 0)) {
            asm volatile("nop" :::);
target_label_3:
            /* Candidate that uses immediate constant (safe) */
            f = safe_add(e, 5);  /* Constant addition won't trap */
            
            /* Inner conditional to create more labels */
            if (__builtin_expect(d > c, 1)) {
                asm volatile("nop" :::);
inner_label:
                /* Another candidate instruction */
                a = safe_add(b, c);
            }
        }
        
        /* Pattern 4: Switch-like pattern with multiple labels */
        switch (i % 4) {
            case 0:
                if (__builtin_expect(a == 0, 0)) {
                    asm volatile("nop" :::);
case0_label:
                    b = safe_add(c, d);
                }
                break;
            case 1:
                if (__builtin_expect(b > 50, 1)) {
                    asm volatile("nop" :::);
case1_label:
                    c = safe_add(d, e);
                }
                break;
            default:
                /* Default case with its own branch */
                if (__builtin_expect(f < 100, 0)) {
                    asm volatile("nop" :::);
default_label:
                    d = safe_add(e, f);
                }
        }
        
        /* Update variables to change branch outcomes */
        a = safe_add(a, 1);
        b = safe_add(b, -1);
        c = safe_add(c, i);
        d = safe_add(d, seed);
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Accumulate results to keep computations live */
        result += a + b + c + d + e + f;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with function calls to create more complex patterns */
    {
        REGISTER_VAR(x, "$s0") = 10;
        REGISTER_VAR(y, "$s1") = 20;
        REGISTER_VAR(z, "$s2") = 30;
        REGISTER_VAR(w, "$s3") = 40;
        
        /* Loop with varying branch density */
        for (int j = 0; j < 50; j++) {
            /* Mix of likely and unlikely branches */
            if (__builtin_expect(x > y, 0)) {
                asm volatile("nop" :::);
label_mix1:
                z = safe_add(w, x);  /* Candidate */
            }
            
            if (__builtin_expect(y < z, 1)) {
                asm volatile("nop" :::);
                asm volatile("nop" :::);
label_mix2:
                w = safe_add(x, y);  /* Candidate */
            }
            
            /* Chain of conditions */
            if (__builtin_expect(z != 0, 1)) {
                if (__builtin_expect(w > 0, 0)) {
                    asm volatile("nop" :::);
chain_label:
                    x = safe_add(y, z);  /* Candidate */
                }
            }
            
            /* Update variables */
            x = safe_add(x, j);
            y = safe_add(y, -j);
            z = (z * 3) / 2;  /* Still safe - no division by zero */
            w = w ^ j;         /* Bitwise operation - no trap */
        }
        
        result += x + y + z + w;
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
