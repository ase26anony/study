/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Barrier to prevent unwanted optimizations */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Create a simple jump pattern */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = b * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot filling */
    /* Uses temporary variable not live across the jump */
    int temp = a + 1;
    result = temp;
    
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int val = x;
    
    /* Force a conditional jump */
    if (x > y) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    val = y - x;
    return val;
    
target2:
    /* Safe arithmetic instruction - no memory access, no function calls */
    /* Uses fresh variable to avoid resource conflicts */
    int local = val;
    local = local ^ 0xFF;  /* Bitwise operation */
    val = local;
    
    return val;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int res = a;
    
    /* Different condition to create varied control flow */
    if ((a & 1) == 0) {  /* If a is even */
        COMPILER_BARRIER();
        goto target3;
    }
    
    res = b + c;
    return res;
    
target3:
    /* Sequence of simple instructions */
    int t1 = res + 1;
    int t2 = t1 * 2;
    res = t2 - 1;
    
    return res;
}

/* Test 4: Nested jumps to create complex flow */
int test_nested_jump_pattern(int val) {
    int result = val;
    
    if (val < 100) {
        if (val > 50) {
            COMPILER_BARRIER();
            goto target4;
        }
        result = val * 3;
        return result;
    }
    
    result = val / 2;
    return result;
    
target4:
    /* Very simple instruction - ideal for delay slot */
    result = result | 0x01;  /* Set lowest bit */
    
    return result;
}

/* Test 5: Jump with register-only operations */
int test_register_only(int a, int b) {
    volatile int trigger = a;  /* Prevent optimization */
    int output = b;
    
    if (trigger != 0) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    output = a + b;
    return output;
    
target5:
    /* Pure register-to-register operation */
    output = output << 2;  /* Shift left by 2 */
    
    return output;
}

/* Test 6: Function with return jump simulation */
int test_return_like_jump(int x) {
    int retval = x;
    
    /* Pattern resembling a return */
    if (x == 0) {
        COMPILER_BARRIER();
        goto early_exit;
    }
    
    retval = x * x;
    return retval;
    
early_exit:
    /* Safe instruction that doesn't use return address register */
    retval = retval + 100;
    
    return retval;
}

/* Main driver that exercises all test patterns */
int main(void) {
    int checksum = 0;
    int i;
    
    /* Run tests with various inputs to explore different paths */
    for (i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i+1);
        checksum += test_conditional_jump(i, i*2);
        checksum += test_multiple_candidates(i, i+2, i+3);
        checksum += test_nested_jump_pattern(i*10);
        checksum += test_register_only(i, i+4);
        checksum += test_return_like_jump(i);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO (generic fallback)");
    
    /* Verify some expected results */
    if (test_unconditional_jump(5, 10) != 6) {
        printf("Warning: test_unconditional_jump gave unexpected result\n");
    }
    
    if (test_conditional_jump(10, 5) != (10 ^ 0xFF)) {
        printf("Warning: test_conditional_jump gave unexpected result\n");
    }
    
    return 0;
}
