/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force variables into specific registers to avoid resource conflicts */
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
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* 
         * BRANCH 1: Simple conditional with predictable outcome
         * Uses __builtin_expect to guide branch prediction
         */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" :::);
            /* Target label for branch - instruction here should be eligible */
target1:
            /* Simple arithmetic - uses registers NOT involved in branch condition */
            e = f + 1;  /* Candidate for delay slot: reg-to-reg operation, no trap */
            /* Continue with other operations */
            result += e;
        }
        
        /* Update variables to change branch outcomes */
        a = b + i;
        b = c - i;
        
        /*
         * BRANCH 2: Different condition, different registers
         * Creates another trial opportunity
         */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);  /* Two nops = more slots to fill */
target2:
            /* Another simple, trap-free operation */
            g = h * 2;  /* Multiplication is safe (no overflow trap in MIPS) */
            result += g;
        }
        
        /* More variable updates */
        c = d ^ i;
        d = e | i;
        
        /*
         * BRANCH 3: Complex enough to require multiple trials
         * The filler will try multiple instructions before finding eligible one
         */
        if (__builtin_expect((e & 1) == 0, 0)) {
            /* Multiple nops to create multiple slots_to_fill */
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
target3:
            /* Perfect delay slot candidate: independent register operation */
            a = b + c;  /* Uses different regs than branch condition (e) */
            result += a;
            
            /* Follow with more instructions that should NOT be candidates */
            f = g + h;  /* This might be tried but uses different resources */
        }
        
        /* Update all variables to create varying patterns */
        e = (f + g) & 0xFF;
        f = (g ^ h) + i;
        g = h << 1;
        h = (h + 1) & 0x7F;  /* Keep in safe range */
        
        /*
         * BRANCH 4: Nested condition to create more complex control flow
         */
        if (__builtin_expect(result < 1000, 1)) {
            if (__builtin_expect(g > 5, 0)) {
                asm volatile("nop" :::);
target4:
                /* Another good candidate */
                d = e + 2;
                result += d;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test: function with multiple return paths */
    test_function(result);
    
    return result > 0 ? 0 : 1;
}

/* Separate function to create more compilation units with branches */
int test_function(int base) {
    register int x asm("$t8");
    register int y asm("$t9");
    register int z asm("$s0");
    
    x = base;
    y = base * 2;
    z = 0;
    
    /* Loop with switch-like structure */
    for (int i = 0; i < 50; i++) {
        switch (i % 4) {
            case 0:
                if (__builtin_expect(x > y, 0)) {
                    asm volatile("nop" :::);
case0_target:
                    z = x + y;  /* Good delay slot candidate */
                }
                break;
            case 1:
                if (__builtin_expect(x < base, 1)) {
                    asm volatile("nop" :::);
case1_target:
                    z = y - x;  /* Another candidate */
                }
                break;
            case 2:
                /* No nop here - tests different slot filling scenarios */
                if (__builtin_expect((x & 1) == 0, 0)) {
case2_target:
                    z = x * 2;  /* Candidate without preceding nop */
                }
                break;
            case 3:
                /* Multiple instructions after label */
                if (__builtin_expect(y > 0, 1)) {
                    asm volatile("nop" :::);
case3_target:
                    z = y + 1;  /* First instruction - good candidate */
                    x = z + 1;  /* Second instruction - might be tried but rejected */
                }
                break;
        }
        
        /* Update variables */
        x = (x + i) & 0xFF;
        y = (y - i) & 0xFF;
    }
    
    return z;
}

/* Inline assembly helper to force specific instruction patterns */
void asm_helpers(void) {
    /* Force use of specific registers in ways that won't conflict */
    register int r1 asm("$16");
    register int r2 asm("$17");
    register int r3 asm("$18");
    
    r1 = 1;
    r2 = 2;
    
    /* Simple conditional that might get delay slot filled */
    if (r1 != r2) {
        asm volatile("nop" :::);
        asm volatile("nop" :::);
asm_target:
        /* Ideal delay slot instruction: uses unrelated register */
        r3 = r1 + 100;
        (void)r3;  /* Use result */
    }
}
