/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Test 1: Simple unconditional jump to label with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        /* This should become a simple jump after optimization */
        goto target1;
    }
    
    /* Dead code to make the jump necessary */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       - Simple arithmetic
       - Uses local variable not live across jump
       - No memory references
       - No function calls
    */
    result = x + 1;  /* Should be safe to move into delay slot */
    
    /* Additional code to prevent tail optimization */
    result += global_a & 0xFF;
    return result;
}

/* Test 2: Conditional jump with safe target instruction */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Create conditional that should optimize to simple jump */
    if (a > b) {
        /* Use asm to prevent certain optimizations */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    temp1 = b - a;
    return temp1 * 2;
    
target2:
    /* Safe instruction: bitwise operation on local temps */
    temp2 = temp1 ^ 0x55;  /* Should not conflict with jump resources */
    
    /* Use result to prevent dead code elimination */
    return temp2 + global_b;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force jump through condition */
    if (val & 1) {
        goto target3;
    }
    
    local3 = val / 3;
    return local3;
    
target3:
    /* Multiple simple instructions - one should be eligible */
    local1 = local1 + 5;      /* Simple addition */
    local2 = local2 | 0xAA;   /* Bitwise OR */
    local3 = local1 - local2; /* Subtraction */
    
    return local3 + global_c;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jump_pattern(int x, int y, int z) {
    int tmp = x;
    
    /* Complex condition that might simplify to simple jump */
    if (x > y && y < z && x != 0) {
        if (z > 10) {
            __asm__ volatile ("# marker1" : : : "memory");
            goto target4;
        }
    }
    
    tmp = y + z;
    return tmp;
    
target4:
    /* Very safe instruction: increment of parameter */
    tmp = x + 1;  /* x is read-only in this path */
    
    /* Force register usage */
    __asm__ volatile ("# marker2" : : : "memory");
    return tmp * 2;
}

/* Test 5: Jump with register-only operations at target */
int test_register_only(int a, int b) {
    register int r1 asm("t0") = a;  /* Suggest temporary register */
    register int r2 asm("t1") = b;
    int result;
    
    /* Unconditional jump pattern */
    if (r1 != 0) {
        goto target5;
    }
    
    result = r2;
    return result;
    
target5:
    /* Pure register-to-register operation */
    result = r1 + r2;  /* Should use only temporary registers */
    
    /* Mix in global to prevent optimization */
    return result + (global_a & 1);
}

/* Test 6: Avoid return address register conflicts (important for MIPS) */
int test_avoid_ra_conflict(int x) {
    int local = x;
    
    /* Don't use return address register in target instruction */
    if (local > 100) {
        goto target6;
    }
    
    return local * 3;
    
target6:
    /* Safe: only uses parameter and constant */
    local = local + 7;  /* Won't conflict with $ra on MIPS */
    
    return local;
}

/* Test 7: Memory operation that should be safe */
int test_safe_memory_op(int *ptr) {
    int value = *ptr;
    int result;
    
    if (value != 0) {
        goto target7;
    }
    
    result = 0;
    return result;
    
target7:
    /* Memory read of known-safe location */
    result = global_a;  /* Global variable access */
    
    /* Simple arithmetic */
    result = result + value;
    return result;
}

/* Portable fallback for non-delay-slot architectures */
int portable_fallback(int seed) {
    return (seed * 1103515245 + 12345) & 0x7FFFFFFF;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int test_data[8] = {5, 12, 8, 20, 3, 150, 7, 9};
    int *ptr = &test_data[0];
    
#if HAS_DELAY_SLOTS
    printf("Testing on architecture with delay slots\n");
    
    /* Run all delay-slot-targeting tests */
    checksum ^= test_unconditional_jump(test_data[0], test_data[1]);
    checksum ^= test_conditional_jump(test_data[1], test_data[2]);
    checksum ^= test_multiple_candidates(test_data[2]);
    checksum ^= test_nested_jump_pattern(test_data[3], test_data[4], test_data[5]);
    checksum ^= test_register_only(test_data[4], test_data[5]);
    checksum ^= test_avoid_ra_conflict(test_data[5]);
    checksum ^= test_safe_memory_op(ptr);
    
    /* Additional test with varying inputs */
    for (int i = 0; i < 8; i++) {
        checksum += test_unconditional_jump(test_data[i], test_data[(i+1)%8]);
    }
#else
    printf("Using portable fallback (no delay slots)\n");
    
    /* Run portable versions */
    for (int i = 0; i < 8; i++) {
        checksum ^= portable_fallback(test_data[i]);
    }
#endif
    
    /* Always run some computation to ensure execution */
    checksum += global_a + global_b + global_c;
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with a simple check */
    if (checksum != 0) {
        printf("Test completed successfully\n");
    } else {
        printf("Warning: checksum is zero\n");
    }
    
    return 0;
}
