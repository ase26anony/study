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
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump to label */
    if (x != 0) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target1;
    }
    
    /* Alternative path */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump context */
    result = x + 1;  /* Uses x which is already in use, but safe */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create a simple conditional jump */
    if (a > b) {
        /* Use separate variables to avoid resource conflicts */
        int unused1 = temp1;
        int unused2 = temp2;
        (void)unused1; (void)unused2;
        
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    result = a + b;
    return result;
    
target2:
    /* Candidate: Simple arithmetic with fresh temporaries */
    int fresh1 = 5;
    int fresh2 = 3;
    result = fresh1 + fresh2;  /* No resource conflicts with jump */
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int res = val;
    
    if (val % 2 == 0) {
        __asm__ volatile ("" : : : "memory");
        goto target3;
    }
    
    res = val * 3;
    return res;
    
target3:
    /* Multiple simple instructions that could be moved */
    int t1 = res;
    int t2 = 7;
    t1 = t1 & 0xFF;      /* Bitwise operation - safe */
    t2 = t2 | 0x10;      /* Another bitwise operation */
    res = t1 + t2;
    return res;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int result = x;
    
    if (x > y) {
        if (y > z) {
            __asm__ volatile ("" : : : "memory");
            goto target4a;
        } else {
            __asm__ volatile ("" : : : "memory");
            goto target4b;
        }
    }
    
    result = z;
    return result;
    
target4a:
    /* Safe arithmetic with constants only */
    result = 42 + 17;
    return result;
    
target4b:
    /* Logical operation */
    result = (x & y) | z;
    return result;
}

/* Test 5: Function with register-sensitive operations */
int test_register_safety(void) {
    int a = global_a;
    int b = global_b;
    int c = global_c;
    int result;
    
    /* Complex condition to force jump generation */
    if ((a ^ b) > c) {
        /* Use volatile to prevent optimization */
        __asm__ volatile ("" : : : "memory");
        goto target5;
    }
    
    result = a * b + c;
    return result;
    
target5:
    /* Instruction that should be safe to move:
       Uses only local temporaries, no globals */
    int t1 = 100;
    int t2 = 200;
    result = t1 - t2;
    return result;
}

/* Test 6: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit with jump */
        if (sum > 1000) {
            __asm__ volatile ("" : : : "memory");
            goto target6;
        }
    }
    
    return sum;
    
target6:
    /* Simple increment - good delay slot candidate */
    sum = sum + 1;
    return sum;
}

/* Test 7: Switch statement with default jump */
int test_switch_jump(int code) {
    int result = 0;
    
    switch (code) {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            break;
        default:
            /* Jump to label from default case */
            __asm__ volatile ("" : : : "memory");
            goto target7;
    }
    
    return result;
    
target7:
    /* Arithmetic with immediate values */
    result = 30 + 40;
    return result;
}

/* Portable fallback for non-delay-slot architectures */
int portable_fallback(int x) {
    return x * 2 + 1;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run test suite multiple times with different inputs */
    for (i = 0; i < 3; i++) {
        checksum += test_unconditional_jump(i, i+1);
        checksum += test_conditional_jump(i*10, i*5);
        checksum += test_multiple_candidates(i*7);
        checksum += test_nested_jumps(i, i+1, i+2);
        checksum += test_register_safety();
        checksum += test_loop_exit(i+5);
        checksum += test_switch_jump(i % 3);
        
#if !HAS_DELAY_SLOTS
        /* Use portable version on non-delay-slot archs */
        checksum += portable_fallback(i);
#endif
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    /* Verify checksum is non-zero (prevents dead code elimination) */
    if (checksum == 0) {
        printf("WARNING: All computations optimized away!\n");
        return 1;
    }
    
    return 0;
}
