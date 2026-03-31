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

/* Barrier to prevent reordering across jumps */
#define JUMP_BARRIER() __asm__ volatile("" ::: "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple unconditional jump to label */
    if (x != 0) {
        JUMP_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Should use register not live across jump */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create a simple conditional jump */
    if (temp1 > temp2) {
        JUMP_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Candidate: Logical operation with temporaries */
    result = temp1 & 0xFF;  /* Mask operation - no memory access */
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int result = 0;
    
    /* Use volatile to force register usage */
    volatile int trigger = global_a;
    
    if (trigger > 20) {
        JUMP_BARRIER();
        goto target3;
    }
    
    result = local1 + local2;
    return result;
    
target3:
    /* Multiple simple instructions that could be moved */
    local1 = local1 + 3;    /* First candidate */
    local2 = local2 | 0x01; /* Second candidate - bitwise OR */
    result = local1 + local2;
    return result;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jump_pattern(int x) {
    int tmp = x;
    
    if (tmp > 10) {
        if (tmp < 100) {
            JUMP_BARRIER();
            goto target4;
        }
        return tmp * 2;
    }
    
    return tmp / 2;
    
target4:
    /* Simple increment - good candidate for delay slot */
    tmp = tmp + 1;
    return tmp;
}

/* Test 5: Jump with register-only operations at target */
int test_register_only(int a, int b) {
    /* Use explicit register variables to control allocation */
    register int r1 asm("t0") = a;
    register int r2 asm("t1") = b;
    int result;
    
    if (r1 != r2) {
        JUMP_BARRIER();
        goto target5;
    }
    
    result = r1 * r2;
    return result;
    
target5:
    /* Register-to-register operation only */
    result = r1 ^ r2;  /* XOR operation - no memory, no function calls */
    return result;
}

/* Test 6: Avoid using return address register (important for MIPS $ra) */
int test_no_ra_conflict(int x) {
    int local = x;
    
    /* Don't use function calls before the jump */
    if (local > 0) {
        JUMP_BARRIER();
        goto target6;
    }
    
    return -local;
    
target6:
    /* Safe instruction: shift operation */
    local = local << 2;  /* Left shift - no trapping */
    return local;
}

/* Test 7: Complex pattern that might trigger maybe_never logic */
int test_complex_pattern(int a, int b, int c) {
    int t1 = a;
    int t2 = b;
    int t3 = c;
    
    /* Multiple conditions to create interesting control flow */
    if ((t1 > t2) && (t2 < t3)) {
        JUMP_BARRIER();
        goto target7;
    }
    
    if (t1 == t2) {
        return t3;
    }
    
    return t1 + t2 + t3;
    
target7:
    /* Very safe instruction: bitwise AND with constant */
    t1 = t1 & 0x7FFFFFFF;  /* Clear sign bit - cannot trap */
    return t1 + t2;
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_controlled(void) {
    int a = global_a;
    int b = global_b;
    int result;
    
    /* Force a simple jump pattern using inline asm */
    __asm__ volatile (
        "move $t0, %1\n\t"           /* Load a into temp reg */
        "move $t1, %2\n\t"           /* Load b into temp reg */
        "bne $t0, $t1, 1f\n\t"       /* Conditional branch */
        "nop\n\t"                    /* Delay slot (might get filled) */
        "move %0, $t0\n\t"           /* Default result */
        "b 2f\n\t"
        "nop\n"
        "1:\n\t"                     /* Target label */
        "addiu $t0, $t0, 1\n\t"      /* Candidate for delay slot */
        "move %0, $t0\n"
        "2:\n"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "t0", "t1", "memory"
    );
    
    return result;
}
#endif

/* Portable fallback for non-delay-slot architectures */
#if !HAS_DELAY_SLOTS
int test_asm_controlled(void) {
    /* Simulate the same logic without relying on delay slots */
    int a = global_a;
    int b = global_b;
    
    if (a != b) {
        return a + 1;
    }
    return a;
}
#endif

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run test 1 */
    checksum += test_unconditional_jump(10, 20);
    printf("Test 1: %d\n", test_unconditional_jump(10, 20));
    
    /* Run test 2 */
    checksum += test_conditional_jump(30, 15);
    printf("Test 2: %d\n", test_conditional_jump(30, 15));
    
    /* Run test 3 */
    checksum += test_multiple_candidates(25);
    printf("Test 3: %d\n", test_multiple_candidates(25));
    
    /* Run test 4 */
    checksum += test_nested_jump_pattern(50);
    printf("Test 4: %d\n", test_nested_jump_pattern(50));
    
    /* Run test 5 */
    checksum += test_register_only(7, 13);
    printf("Test 5: %d\n", test_register_only(7, 13));
    
    /* Run test 6 */
    checksum += test_no_ra_conflict(8);
    printf("Test 6: %d\n", test_no_ra_conflict(8));
    
    /* Run test 7 */
    checksum += test_complex_pattern(5, 10, 15);
    printf("Test 7: %d\n", test_complex_pattern(5, 10, 15));
    
    /* Run architecture-specific test */
    checksum += test_asm_controlled();
    printf("ASM test: %d\n", test_asm_controlled());
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are consistent */
    if (checksum != test_unconditional_jump(10, 20) +
                    test_conditional_jump(30, 15) +
                    test_multiple_candidates(25) +
                    test_nested_jump_pattern(50) +
                    test_register_only(7, 13) +
                    test_no_ra_conflict(8) +
                    test_complex_pattern(5, 10, 15) +
                    test_asm_controlled()) {
        printf("ERROR: Checksum mismatch!\n");
        return 1;
    }
    
    printf("All tests completed successfully.\n");
    return 0;
}
