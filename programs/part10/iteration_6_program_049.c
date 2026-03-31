/* reorg_delay_slot_test.c
 * Designed to trigger GCC's delay slot filler logic in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force variables into specific registers to avoid resource conflicts */
register int a asm("$2");
register int b asm("$3");
register int c asm("$4");
register int d asm("$5");
register int e asm("$6");
register int f asm("$7");
register int g asm("$8");
register int h asm("$9");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    int result = 0;
    
    /* Initialize registers with values that will create varying branch outcomes */
    a = 0;
    b = 1;
    c = 2;
    d = 3;
    e = 4;
    f = 5;
    g = 6;
    h = 7;
    
    /* Main loop - prevents optimization and preserves branch structure */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Conditional branch with predictable pattern */
        /* Use __builtin_expect to create a branch that's predictable but not always taken */
        if (__builtin_expect((a > b), 0)) {
            /* Insert nop to create a filler slot candidate */
            asm volatile("nop" ::: "memory");
            /* Target label for the branch */
            target_label_1:
            /* ELIGIBLE DELAY SLOT CANDIDATE: Simple arithmetic with registers
               not used in branch condition (c,d vs a,b) */
            c = d + 1;  /* This should be a simple register-to-register operation */
        } else {
            /* Alternative path to ensure both branches exist */
            a = b + 1;
        }
        
        /* BRANCH 2: Different condition to create multiple trial opportunities */
        /* Vary the number of nops to force multiple filling attempts */
        if (__builtin_expect((c < d), 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops - more slots to fill */
            target_label_2:
            /* Another eligible candidate using different registers */
            e = f + 2;  /* Simple arithmetic, no traps, no resource conflicts */
        }
        
        /* BRANCH 3: Complex pattern with multiple potential candidates */
        /* Create a scenario where slots_to_fill != slots_filled */
        if (__builtin_expect((e % 3) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three slots to potentially fill */
            target_label_3:
            /* Multiple independent operations that could be candidates */
            g = h + a;  /* Uses h and a, but a was updated earlier - still safe */
            /* Followed by another simple operation */
            f = g - 1;  /* Simple arithmetic, register only */
        }
        
        /* BRANCH 4: Nested condition to create more complex control flow */
        if (__builtin_expect((g > 0), 1)) {
            /* No nop here - tests case with immediate filling opportunity */
            target_label_4:
            /* Very simple candidate - immediate constant operation */
            h = 42;  /* Move immediate - should be a single RTL instruction */
        }
        
        /* Update variables to change branch outcomes in next iteration */
        a = (a + 1) % 10;
        b = (b + 2) % 10;
        c = (c * 2) % 100;
        d = (d + i) % 100;
        
        /* Use results to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h;
    }
    
    /* Force output to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional test: Function with inline assembly to create specific patterns */
    test_delay_slot_pattern();
    
    return result > 0 ? 0 : 1;
}

/* Separate function to create isolated delay slot opportunities */
void test_delay_slot_pattern(void) {
    register int x asm("$10");
    register int y asm("$11");
    register int z asm("$12");
    register int w asm("$13");
    
    x = 100;
    y = 200;
    z = 300;
    w = 400;
    
    volatile int count = 50;
    
    for (int j = 0; j < count; j++) {
        /* Pattern 1: Branch with single nop before label */
        if (__builtin_expect(x > y, 0)) {
            asm volatile("nop" ::: "memory");
            delay_target_1:
            /* Perfect candidate: uses registers not in condition, simple op */
            z = w + j;  /* w is not x or y, j is loop counter */
        }
        
        /* Pattern 2: Multiple branches to same label (different paths) */
        if (__builtin_expect((x + y) < 500, 1)) {
            goto delay_target_2;
        } else {
            asm volatile("nop" ::: "memory");
            delay_target_2:
            /* Candidate that sets a register not used elsewhere */
            asm volatile("# Candidate instruction" : "=r"(x) : "0"(y + 1));
        }
        
        /* Pattern 3: Switch-like structure with multiple labels */
        switch (j % 3) {
            case 0:
                if (__builtin_expect(z > 250, 1)) {
                    asm volatile("nop" ::: "memory");
                    case_label_0:
                    w = x * 2;  /* Multiplication is still trap-free for integers */
                }
                break;
            case 1:
                if (__builtin_expect(w < 600, 0)) {
                    asm volatile("nop" ::: "memory");
                    case_label_1:
                    x = y | 0xFF;  /* Bitwise operation - safe, no traps */
                }
                break;
        }
        
        /* Update variables */
        x = (x + j) & 0xFFF;
        y = (y - j) & 0xFFF;
        z ^= j;  /* XOR operation */
    }
    
    /* Prevent function from being optimized away */
    asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
}

/* Helper function to create register pressure */
static inline int safe_arithmetic(int p1, int p2, int p3) {
    /* Only register operations, no memory access, no division */
    return (p1 + p2) * p3 - (p1 & p2) | (p3 ^ 0x1234);
}

/* Additional test case focusing on the specific conditions */
void specific_condition_test(void) {
    /* This function aims specifically at the conditions in lines 2135-2149 */
    register int r0 asm("$14");
    register int r1 asm("$15");
    register int r2 asm("$16");
    register int r3 asm("$17");
    register int r4 asm("$18");
    
    r0 = 0;
    r1 = 1;
    r2 = 2;
    r3 = 3;
    r4 = 4;
    
    /* Create a predictable branch with clear label target */
    if (__builtin_expect(r0 != r1, 1)) {
        /* Multiple nops to create fillable slots */
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
        /* The label must be the target of a simple jump */
        specific_label:
        /* The perfect candidate:
           - Non-jump instruction
           - Not a SEQUENCE
           - No resource conflicts (uses r2,r3,r4, not r0,r1)
           - No traps (simple addition)
           - Can be split if needed
        */
        r4 = r2 + r3;
        
        /* Follow with more simple instructions */
        r2 = r4 - r3;
        r3 = r2 << 2;  /* Shift operation - still trap-free */
    }
    
    /* Force the label to be referenced (prevent optimization) */
    if (r4 > 100) {
        goto specific_label;
    }
}
