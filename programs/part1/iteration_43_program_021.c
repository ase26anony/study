/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local temporaries not live across the jump */
    int temp1 = x + 1;      /* Should be safe to move */
    int temp2 = y - 1;
    result = temp1 + temp2;
    
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int sum = 0;
    
    /* Create a conditional jump */
    if (a > b) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    sum = a + b;
    return sum;
    
target2:
    /* Candidate: bitwise operations on fresh temporaries */
    /* These shouldn't conflict with jump resources */
    int t1 = a ^ 0xFF;
    int t2 = b & 0x0F;
    sum = t1 | t2;
    
    return sum;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int output = val;
    
    if (val < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    output = val * 3;
    return output;
    
target3:
    /* Multiple simple instructions - compiler might choose one */
    int tmp1 = val << 2;    /* Shift operation */
    int tmp2 = tmp1 + 7;    /* Addition */
    int tmp3 = tmp2 ^ val;  /* XOR */
    output = tmp3;
    
    return output;
}

/* Test 4: Jump that returns from function (tests return address handling) */
int test_with_return_like(int x, int y) {
    int res = x;
    
    /* Make this look return-like but not actually return */
    if (x == y) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    res = x - y;
    return res;
    
target4:
    /* Safe instruction: doesn't touch special registers */
    int safe_op = (x * 2) + (y / 2);
    res = safe_op;
    
    return res;
}

/* Test 5: Nested jumps to create more complex patterns */
int test_nested_pattern(int a, int b, int c) {
    int val = a;
    
    if (a > b) {
        if (b > c) {
            COMPILER_BARRIER();
            goto target5;
        }
        val = a + c;
    }
    
    return val;
    
target5:
    /* Very safe instruction: only uses immediate values */
    int safe = 1 + 2 + 3;  /* Compile-time constant expression */
    val = safe + a;        /* Only uses 'a' which was live before jump */
    
    return val;
}

/* Test 6: Jump with memory operation (riskier - may not be eligible) */
int test_with_memory_op(int *ptr, int idx) {
    int value = idx;
    
    if (ptr != NULL) {
        COMPILER_BARRIER();
        goto target6;
    }
    
    value = -1;
    return value;
    
target6:
    /* Memory load - might not be eligible if maybe_never is true */
    int loaded = ptr[idx];  /* Could trap if ptr is bad or idx out of bounds */
    value = loaded + 1;
    
    return value;
}

/* Test 7: Pure arithmetic at target - most likely candidate */
int test_pure_arithmetic(int x, int y, int z) {
    int result = x;
    
    /* Force predictable jump */
    if (z != 0) {
        COMPILER_BARRIER();
        goto target7;
    }
    
    result = y;
    return result;
    
target7:
    /* Ideal candidate: pure arithmetic, no side effects */
    /* Uses registers that shouldn't be in 'set' or 'needed' */
    int arith1 = (x + y) * 2;
    int arith2 = arith1 - z;
    int arith3 = arith2 & 0x7FFFFFFF;
    result = arith3;
    
    return result;
}

/* Test 8: Jump across basic block boundary */
int test_cross_block(int a, int flag) {
    int r = a;
    
    switch (flag & 3) {
        case 0:
            r = a * 2;
            break;
        case 1:
            COMPILER_BARRIER();
            goto target8;
        default:
            r = a / 2;
            break;
    }
    
    return r;
    
target8:
    /* Simple increment - good candidate */
    r = a + 1;
    return r;
}

/* Main driver that exercises all tests */
int main(void) {
    int checksum = 0;
    int array[4] = {10, 20, 30, 40};
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all test functions with various inputs */
    checksum += test_unconditional_jump(5, 3);
    checksum += test_conditional_jump(global_a, global_b);
    checksum += test_multiple_candidates(50);
    checksum += test_with_return_like(7, 7);
    checksum += test_nested_pattern(10, 5, 2);
    checksum += test_with_memory_op(array, 2);
    checksum += test_pure_arithmetic(2, 3, 4);
    checksum += test_cross_block(8, 1);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results match expectations */
    int expected = test_unconditional_jump(5, 3) +
                   test_conditional_jump(42, 17) +
                   test_multiple_candidates(50) +
                   test_with_return_like(7, 7) +
                   test_nested_pattern(10, 5, 2) +
                   test_with_memory_op(array, 2) +
                   test_pure_arithmetic(2, 3, 4) +
                   test_cross_block(8, 1);
    
    if (checksum == expected) {
        printf("All tests passed!\n");
    } else {
        printf("Warning: Checksum mismatch (got %d, expected %d)\n", 
               checksum, expected);
    }
    
    return 0;
}
