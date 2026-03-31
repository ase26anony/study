/* reorg_delay_slot.c
 * Target: GCC delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-all -fno-gcse -fno-crossjumping -S reorg_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int a asm("$2");
register int b asm("$3");
register int c asm("$4");
register int d asm("$5");
register int e asm("$6");
register int f asm("$7");
register int g asm("$8");
register int h asm("$9");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 1000;

int main() {
    int result = 0;
    
    /* Initialize registers with distinct values */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    
    /* Main loop to force multiple delay slot filling attempts */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        if (__builtin_expect(a < b, 1)) {
            /* Multiple nops to create filler opportunities */
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            /* Target label for branch 1 */
            target1:
            /* ELIGIBLE DELAY SLOT CANDIDATE:
               Simple register-to-register operation, no traps,
               uses registers not involved in branch condition */
            e = f + 1;  /* Independent of a,b - safe to move */
            
            /* Continue with some computation */
            result += e;
        }
        
        /* BRANCH 2: Predictable not-taken branch with different nop pattern */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            target2:
            /* Another eligible candidate using different registers */
            g = h + 2;  /* Independent of c,d */
            
            result -= g;
        }
        
        /* BRANCH 3: Variable condition to create different flow */
        if (__builtin_expect((i & 1) == 0, 0)) {
            asm volatile("nop" :::);
            
            target3:
            /* Simple arithmetic with immediate - trap-free */
            a = b + 3;  /* This reuses a,b but after branch decision */
            
            result ^= a;
        }
        
        /* BRANCH 4: Complex condition with multiple nops */
        if (__builtin_expect(e != f, 1)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            target4:
            /* Independent operation - perfect delay slot candidate */
            c = d * 2;  /* Multiplication is safe (no division) */
            
            result |= c;
        }
        
        /* Modify variables to change branch patterns */
        a += i;
        b -= (i % 3);
        c ^= 0x55;
        d = (d << 1) | 1;
        
        /* Ensure variables stay in bounds */
        if (e > 1000) e = 5;
        if (f < 0) f = 6;
        
        /* Force compiler to keep all variables live */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d),
                        "+r"(e), "+r"(f), "+r"(g), "+r"(h));
    }
    
    /* Additional test case: nested branches */
    {
        register int x asm("$10") = 100;
        register int y asm("$11") = 200;
        register int z asm("$12") = 0;
        
        /* Outer branch */
        if (__builtin_expect(x < y, 1)) {
            asm volatile("nop" :::);
            
            outer_label:
            /* This could be moved to delay slot */
            z = x + y;
            
            /* Inner branch - creates more opportunities */
            if (__builtin_expect(z > 150, 1)) {
                asm volatile("nop" :::);
                asm volatile("nop" :::);
                
                inner_label:
                /* Another eligible instruction */
                x = y - 50;
            }
        }
        
        result += z;
    }
    
    /* Final computation to prevent dead code elimination */
    result = (result & 0xFF) + a + b + c + d + e + f + g + h;
    
    printf("Result: %d\n", result);
    return result;
}

/* Helper function to create more branch opportunities */
static int helper(int n) {
    register int p asm("$16") = n;
    register int q asm("$17") = n * 2;
    register int r asm("$18") = 0;
    
    /* Multiple branches in helper function */
    if (__builtin_expect(p > 0, 1)) {
        asm volatile("nop" :::);
        
        helper_label1:
        r = q + 10;
    }
    
    if (__builtin_expect(r < 100, 1)) {
        asm volatile("nop" :::);
        asm volatile("nop" :::);
        
        helper_label2:
        p = r * 3;
    }
    
    return p + r;
}
