/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Force a simple jump to label */
    if (a != 0) {
        goto target1;
    }
    
    /* Some code that won't be reached if a != 0 */
    b = b * 2;
    return b;
    
target1:
    /* Candidate instruction for delay slot filling */
    /* Uses temporary variable not used before jump */
    int temp = a + 1;
    result = temp;
    
    /* Additional instructions to prevent tail optimization */
    result = result * 3 - 2;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int val = x;
    
    /* Create a conditional jump */
    if (x > y) {
        /* Compiler barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    val = y - x;
    return val;
    
target2:
    /* Safe arithmetic instruction - uses fresh variable */
    int local = x + y;
    local = local & 0xFF;  /* Bitwise operation */
    
    /* More operations to make it non-trivial */
    val = local * 2 + 1;
    return val;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int res = 0;
    
    /* Nested condition to create jump */
    if (a > 0 && b < 10) {
        if (c != 0) {
            goto target3;
        }
    }
    
    res = a + b + c;
    return res;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    int t1 = a << 2;      /* Shift operation */
    int t2 = b | c;       /* Bitwise OR */
    int t3 = t1 ^ t2;     /* Bitwise XOR */
    
    res = t3 + 1;
    return res;
}

/* Test 4: Function with return jump pattern */
int test_return_jump(int *ptr, int n) {
    if (n <= 0) {
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (ptr[i] == 0) {
            goto target4;
        }
        sum += ptr[i];
    }
    
    return sum;
    
target4:
    /* Safe instruction that doesn't use special registers */
    int adjustment = n * 2;
    adjustment = adjustment + 1;
    
    return adjustment;
}

/* Test 5: Complex jump network to increase optimization opportunities */
int test_complex_flow(int a, int b, int c) {
    volatile int trigger = a; /* Prevent optimization */
    
    if (trigger > 100) {
        goto label_a;
    } else if (trigger > 50) {
        goto label_b;
    }
    
    return a + b;
    
label_a:
    /* First candidate block */
    {
        int tmp = b * c;
        tmp = tmp >> 1;
        return tmp + a;
    }
    
label_b:
    /* Second candidate block */
    {
        int tmp = a ^ b ^ c;
        tmp = tmp & 0x7F;
        return tmp * 2;
    }
}

/* Test 6: Avoid resource conflicts by using fresh variables */
int test_fresh_variables(int x) {
    int original = x;
    
    /* Force jump with goto */
    if (x % 2 == 0) {
        goto compute;
    }
    
    return x * 3;
    
compute:
    /* Use completely fresh variables to avoid conflicts */
    int fresh1 = x + 1;
    int fresh2 = fresh1 * 2;
    int fresh3 = fresh2 - x;
    
    return fresh3;
}

/* Portable fallback for architectures without delay slots */
int test_portable(int a, int b) {
    return (a > b) ? (a + b) : (a - b);
}

/* Main driver that calls all tests */
int main(void) {
    int checksum = 0;
    int test_data[6] = {10, 20, 30, 40, 50, 60};
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run architecture-specific tests if applicable */
#if HAS_DELAY_SLOTS
    printf("Compiled for architecture with delay slots\n");
    
    /* Test 1 */
    checksum += test_unconditional_jump(test_data[0], test_data[1]);
    
    /* Test 2 */
    checksum += test_conditional_jump(test_data[1], test_data[2]);
    
    /* Test 3 */
    checksum += test_multiple_candidates(test_data[0], test_data[1], test_data[2]);
    
    /* Test 4 */
    checksum += test_return_jump(test_data, 3);
    
    /* Test 5 */
    checksum += test_complex_flow(test_data[3], test_data[4], test_data[5]);
    
    /* Test 6 */
    checksum += test_fresh_variables(test_data[0]);
#else
    printf("No delay slots on this architecture, using portable tests\n");
    
    /* Portable tests */
    for (int i = 0; i < 5; i++) {
        checksum += test_portable(test_data[i], test_data[i + 1]);
    }
#endif
    
    /* Always run some portable computation */
    checksum += test_portable(100, 200);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
