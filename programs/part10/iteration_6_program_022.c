/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * Or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force variables into specific registers to control resource allocation */
register int cond_a asm("$2");
register int cond_b asm("$3");
register int slot_cand1 asm("$4");
register int slot_cand2 asm("$5");
register int result1 asm("$6");
register int result2 asm("$7");
register int loop_ctr asm("$8");

/* Volatile to prevent optimization of branches */
volatile int iterations = 100;
volatile int seed = 42;

int main(void) {
    int final_result = 0;
    
    /* Initialize registers with distinct values */
    cond_a = seed;
    cond_b = seed * 2;
    slot_cand1 = seed + 1;
    slot_cand2 = seed + 2;
    result1 = 0;
    result2 = 0;
    loop_ctr = 0;
    
    /* Main loop to create multiple delay slot filling opportunities */
    while (loop_ctr < iterations) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        if (__builtin_expect(cond_a > cond_b, 1)) {
            /* This nop creates a filler instruction the compiler may try to replace */
            asm volatile("nop" ::: "memory");
            /* TARGET LABEL 1 */
        target1:
            /* Eligible delay slot candidate: simple arithmetic, no traps,
               uses registers not involved in branch condition ($4, $5, $6) */
            result1 = slot_cand1 + slot_cand2;
            /* Continue after label */
            cond_a -= 1;
        }
        
        /* BRANCH 2: Predictable not-taken branch with different register usage */
        if (__builtin_expect(cond_b < cond_a, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple trial attempts */
        target2:
            /* Another eligible candidate using different registers */
            result2 = slot_cand1 - 5;  /* Safe immediate operation */
            cond_b += 2;
        }
        
        /* BRANCH 3: Variable condition to create different flow patterns */
        if (__builtin_expect((cond_a ^ cond_b) & 1, loop_ctr & 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
        target3:
            /* Mix of operations to test various candidate types */
            slot_cand1 = result1 + result2;  /* Register-to-register move-like operation */
            /* This should not be expanded to SEQUENCE */
        }
        
        /* BRANCH 4: Another pattern with immediate operand */
        if (__builtin_expect(cond_a == cond_b, 0)) {
            asm volatile("nop" ::: "memory");
        target4:
            /* Simple increment - very safe for delay slot */
            slot_cand2 = slot_cand2 + 1;
            /* Force a label reference to ensure label isn't optimized away */
            asm volatile("" : : "r"(&target4));
        }
        
        /* Modify conditions to change branch behavior across iterations */
        cond_a = (cond_a * 1103515245 + 12345) & 0x7fffffff;
        cond_b = (cond_b * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Occasionally swap register usage to test resource conflict detection */
        if ((loop_ctr & 7) == 0) {
            int temp = slot_cand1;
            slot_cand1 = slot_cand2;
            slot_cand2 = temp;
        }
        
        loop_ctr++;
        
        /* Prevent loop unrolling while maintaining live values */
        asm volatile("" : "+r"(cond_a), "+r"(cond_b), 
                          "+r"(slot_cand1), "+r"(slot_cand2),
                          "+r"(result1), "+r"(result2) : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    final_result = result1 + result2 + slot_cand1 + slot_cand2;
    
    /* Additional test case outside loop for single-pass analysis */
    {
        register int test_a asm("$9") = 100;
        register int test_b asm("$10") = 200;
        register int test_c asm("$11") = 0;
        
        /* This creates a simple eligible pattern for the delay slot filler */
        if (__builtin_expect(test_a < test_b, 1)) {
            asm volatile("nop" ::: "memory");
        final_target:
            /* Perfect candidate: uses registers not in condition, no trap */
            test_c = test_a + 10;  /* $11 = $9 + 10 */
        }
        
        final_result += test_c;
    }
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}

/* Helper function to create additional context (optional) */
static void __attribute__((noinline)) 
create_more_context(int x, int y) {
    /* This creates more RTL context but doesn't interfere with 
       the main delay slot filling opportunities */
    register int a asm("$16") = x;
    register int b asm("$17") = y;
    register int c asm("$18");
    
    if (__builtin_expect(a != b, 0)) {
        asm volatile("nop" ::: "memory");
    extra_label:
        c = a * 2 + b;  /* Still trap-free arithmetic */
    }
    
    /* Reference label to keep it alive */
    asm volatile("" : : "r"(&extra_label));
}
