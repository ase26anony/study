/* test_delay_slots.c
 * Designed to trigger GCC's delay slot filling logic in reorg.cc
 * Specifically targets lines 2135-2149 in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Create a simple jump pattern */
    if (a != 0) {
        /* Force a simple jump to label */
        goto target_label;
    }
    
    /* This path should not be taken */
    result = b * 2;
    return result;
    
target_label:
    /* Candidate instruction for delay slot filling:
     * Simple arithmetic that doesn't conflict with jump resources
     * Using local variable that's not live across the jump
     */
    int temp = a + 1;  /* Should use register not needed by jump */
    result = temp;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int val = x;
    
    /* Create conditional that leads to simple jump */
    if (x > y) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        goto cond_target;
    }
    
    /* Alternative path */
    val = y - x;
    return val;
    
cond_target:
    /* Safe instruction: bitwise operation on local variable */
    val = val ^ 0xFF;  /* XOR with constant - safe, no memory access */
    return val;
}

/* Test 3: Multiple jumps with different patterns */
int test_multiple_patterns(int a, int b, int c) {
    int result = 0;
    
    /* Pattern 1: Jump based on modulo */
    if ((a % 2) == 0) {
        goto pattern1_target;
    }
    
    /* Pattern 2: Jump based on bit test */
    if (b & 0x01) {
        goto pattern2_target;
    }
    
    result = c;
    return result;
    
pattern1_target:
    /* Simple increment - good candidate for delay slot */
    result = a + 1;
    /* Force another jump to create more opportunities */
    if (result > 10) {
        goto final_target;
    }
    return result;
    
pattern2_target:
    /* Logical operation - also good candidate */
    result = b | 0x80;
    return result;
    
final_target:
    /* Another candidate instruction */
    result = result * 2;
    return result;
}

/* Test 4: Nested jumps with safe arithmetic */
int test_nested_jumps(int p, int q) {
    int tmp = p;
    
    if (p < 100) {
        if (q > 50) {
            /* This should create a simple jump */
            goto inner_target;
        }
        tmp = p + q;
    }
    
    tmp = tmp * 2;
    return tmp;
    
inner_target:
    /* Very safe instruction: register-only operation */
    tmp = tmp + 5;  /* Using tmp which is set before the jump */
    return tmp;
}

/* Test 5: Avoid resource conflicts explicitly */
int test_no_conflicts(int base, int offset) {
    /* Use local variables that won't conflict with jump resources */
    int local1 = base;
    int local2 = offset;
    int result;
    
    /* Create jump condition */
    if (local1 != 0) {
        /* Use asm to ensure jump is generated */
        __asm__ volatile("" : : : "memory");
        goto safe_target;
    }
    
    result = local2;
    return result;
    
safe_target:
    /* Instruction designed to avoid resource conflicts:
     * - Uses only local variables
     * - No memory access (no trapping)
     * - Simple arithmetic
     */
    result = local1 + local2;
    
    /* Additional safe instruction */
    result = result & ~0x03;
    return result;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int val) {
    int adjusted = val;
    
    if (adjusted > 1000) {
        /* This may create a return jump */
        goto adjust_and_return;
    }
    
    return adjusted * 2;
    
adjust_and_return:
    /* Safe instruction before return */
    adjusted = adjusted - 100;
    return adjusted;
}

/* Driver function that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    /* Seed for reproducible behavior */
    srand(42);
    
    /* Run multiple iterations with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        checksum += test_unconditional_jump(a, b);
        checksum += test_conditional_jump(b, c);
        checksum += test_multiple_patterns(a, b, c);
        checksum += test_nested_jumps(a, c);
        checksum += test_no_conflicts(b, c);
        checksum += test_return_jump(a * 10);
        
        /* Architecture-specific optimization hint */
        #if HAS_DELAY_SLOTS
        /* Insert compiler barrier to prevent over-optimization */
        __asm__ volatile("" : : : "memory");
        #endif
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "Yes" : "No (generic fallback)");
    
    /* Verify with a simple test case */
    int test_val = test_unconditional_jump(10, 20);
    printf("Test value: %d (expected 11)\n", test_val);
    
    return 0;
}
