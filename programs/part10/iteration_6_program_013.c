/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid resource conflicts */
register int reg_a asm("$2");  /* Branch condition variable 1 */
register int reg_b asm("$3");  /* Branch condition variable 2 */
register int reg_c asm("$4");  /* Candidate instruction source 1 */
register int reg_d asm("$5");  /* Candidate instruction source 2 */
register int reg_e asm("$6");  /* Candidate instruction destination */
register int reg_f asm("$7");  /* Another independent register */

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Function to create predictable branch patterns */
int process_value(int x) {
    return (x * 3 + 7) & 0xFF;
}

int main() {
    int i, result = 0;
    
    /* Initialize register variables */
    reg_a = 0;
    reg_b = 100;
    reg_c = 1;
    reg_d = 2;
    reg_e = 0;
    reg_f = 0;
    
    /* Main loop with multiple branch patterns */
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Branch with single nop before target */
        if (__builtin_expect(reg_a < reg_b, 1)) {
            asm volatile("nop" ::: "memory");
            /* Target label for delay slot candidate */
target_label_1:
            /* Simple arithmetic - eligible for delay slot */
            reg_e = reg_c + reg_d;  /* Uses different registers than branch condition */
        }
        
        /* Pattern 2: Branch with multiple nops (forces multiple trials) */
        if (__builtin_expect(reg_a > (reg_b / 2), 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_2:
            /* Another simple arithmetic operation */
            reg_f = reg_d - reg_c;
        }
        
        /* Pattern 3: Branch with varying condition */
        if (__builtin_expect((reg_a & 1) == 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_3:
            /* Different arithmetic operation */
            reg_e = reg_c * 2;
        }
        
        /* Pattern 4: More complex branch pattern */
        if (__builtin_expect(reg_a != reg_b, 0)) {
            /* Multiple filler instructions */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_4:
            /* Safe arithmetic - no traps, no memory access */
            reg_f = reg_d + 5;  /* Immediate constant is safe */
        }
        
        /* Update variables to change branch behavior */
        reg_a = process_value(reg_a + i);
        reg_b = process_value(reg_b - i);
        
        /* Alternate between different register sets for candidate instructions */
        if (i & 1) {
            reg_c = reg_a + 1;
            reg_d = reg_b - 1;
        } else {
            reg_c = reg_b + 2;
            reg_d = reg_a - 2;
        }
        
        /* Accumulate result to keep computation live */
        result += reg_e + reg_f;
    }
    
    /* Additional patterns outside loop to increase coverage */
    
    /* Pattern with immediate branch target */
    reg_a = 50;
    reg_b = 100;
    if (__builtin_expect(reg_a < reg_b, 1)) {
        asm volatile("nop" ::: "memory");
target_label_5:
        /* Very simple operation - high chance of being moved */
        reg_e = reg_c + 1;
    }
    
    /* Pattern with opposite prediction */
    if (__builtin_expect(reg_a > reg_b, 0)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
target_label_6:
        reg_f = reg_d - 1;
    }
    
    /* Final computation using all modified registers */
    result += reg_e * reg_f;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
