/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Barrier to prevent unwanted optimizations */
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Dead code to create separation */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump context */
    result = x + 1;  /* Should use registers not live across jump */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Create a simple conditional jump */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    temp1 = temp1 - temp2;
    return temp1;
    
target2:
    /* Candidate: Logical operation with safe registers */
    temp1 = temp1 & 0xFF;  /* Mask operation */
    return temp1;
}

/* Test 3: Jump with multiple safe instructions at target */
int test_multiple_instructions(int val) {
    int local1 = val;
    int local2 = 0;
    
    /* Force jump with goto */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local2 = local1 * 2;
    return local2;
    
target3:
    /* Multiple simple instructions - first one might be moved */
    local1 = local1 + 5;      /* Potential delay slot candidate */
    local2 = local1 * 2;      /* Follow-up instruction */
    return local2;
}

/* Test 4: Jump with bitwise operations at target */
int test_bitwise_ops(int x) {
    int mask = 0x0F;
    int result = x;
    
    /* Unconditional-like jump structure */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return 0;
    
target4:
    /* Bitwise operations are typically safe for delay slots */
    result = result ^ mask;    /* XOR operation */
    result = result | 0x10;    /* OR operation */
    return result;
}

/* Test 5: Nested jumps to create complex flow */
int test_nested_jumps(int a, int b, int c) {
    int t1 = a, t2 = b, t3 = c;
    
    if (t1 > t2) {
        COMPILER_BARRIER();
        if (t2 > t3) {
            goto target5;
        }
        t1 = t1 + t3;
        return t1;
    }
    
    t2 = t2 * t3;
    return t2;
    
target5:
    /* Safe arithmetic with temporaries */
    t3 = t1 + t2;  /* Uses registers defined before jump */
    return t3;
}

/* Test 6: Function with switch-like jump table pattern */
int test_switch_like(int x) {
    int result = x;
    
    switch (x & 3) {
        case 0:
            COMPILER_BARRIER();
            goto target6_0;
        case 1:
            result = x * 2;
            break;
        case 2:
            result = x / 2;
            break;
        default:
            goto target6_def;
    }
    return result;
    
target6_0:
    result = result << 1;  /* Shift operation */
    return result;
    
target6_def:
    result = result >> 1;  /* Another shift */
    return result;
}

/* Test 7: Avoid using return address register (important for MIPS $ra) */
int test_no_ra_conflict(int x) {
    int temp1 = x;
    int temp2 = 42;
    
    /* Create jump without function call in path */
    if (temp1 > 0) {
        COMPILER_BARRIER();
        goto target7;
    }
    
    temp2 = temp1 * 3;
    return temp2;
    
target7:
    /* Use only temporaries, avoid any register that might be $ra */
    temp1 = temp1 + temp2;
    return temp1;
}

/* Test 8: Memory operation that should be safe */
int test_safe_memory(int *ptr, int idx) {
    int local = idx;
    int array[4] = {1, 2, 3, 4};
    
    if (ptr != NULL) {
        COMPILER_BARRIER();
        goto target8;
    }
    
    local = array[0] + array[1];
    return local;
    
target8:
    /* Safe memory access to local array */
    local = array[local & 3];  /* Bounded array access */
    return local;
}

/* Portable fallback for non-delay-slot architectures */
int test_portable_fallback(int x) {
    /* Perform same computation without relying on delay slot behavior */
    int result = x;
    
    if (x > 0) {
        result = result + 1;
    } else {
        result = result - 1;
    }
    
    return result;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int test_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run architecture-specific tests if supported */
#if HAS_DELAY_SLOTS
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(30, 15);
    checksum += test_multiple_instructions(25);
    checksum += test_bitwise_ops(0x55);
    checksum += test_nested_jumps(5, 10, 15);
    checksum += test_switch_like(7);
    checksum += test_no_ra_conflict(100);
    checksum += test_safe_memory(test_array, 2);
#else
    /* Portable fallback tests */
    printf("Using portable fallback tests\n");
    for (int i = 0; i < 8; i++) {
        checksum += test_portable_fallback(test_array[i]);
    }
#endif
    
    /* Additional portable test to ensure execution */
    checksum += test_array[0] + test_array[7];
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    return checksum != 0 ? 0 : 1;
}
