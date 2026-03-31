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
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Force a simple unconditional jump pattern */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    return b * 2;
    
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
    
    /* Create a conditional jump */
    if (x > y) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Fall through path */
    val = y - x;
    return val;
    
target2:
    /* Safe arithmetic instruction - no memory ops, no function calls */
    /* Uses fresh variable to avoid resource conflicts */
    int delta = 3;
    val = x + delta;
    
    return val;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_instructions(int a, int b, int c) {
    int res = a;
    
    /* Force jump based on parameter */
    if (c > 0) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    /* Alternative path */
    res = b;
    return res;
    
target3:
    /* Sequence of simple instructions that could be moved */
    int t1 = a & 0xFF;      /* Bitwise operation */
    int t2 = t1 | 0x80;     /* Another bitwise op */
    res = t2 + b;           /* Arithmetic */
    
    return res;
}

/* Test 4: Nested jumps to create more complex patterns */
int test_nested_jumps(int val) {
    int output = val;
    
    if (val < 100) {
        COMPILER_BARRIER();
        goto outer_target;
    }
    
    return output * 2;
    
outer_target:
    /* First instruction at target - potential delay slot candidate */
    int modified = val + 10;
    
    /* Another conditional jump */
    if (modified > 50) {
        COMPILER_BARRIER();
        goto inner_target;
    }
    
    output = modified;
    return output;
    
inner_target:
    /* Another candidate instruction */
    output = modified * 2;
    return output;
}

/* Test 5: Function with switch statement creating multiple jumps */
int test_switch_jumps(int code) {
    int result = 0;
    
    switch (code & 0x3) {
        case 0:
            COMPILER_BARRIER();
            goto case_target0;
        case 1:
            result = 1;
            break;
        case 2:
            result = 2;
            break;
        case 3:
            COMPILER_BARRIER();
            goto case_target3;
        default:
            return -1;
    }
    
    return result;
    
case_target0:
    /* Safe arithmetic at jump target */
    result = (code << 1) + 5;
    return result;
    
case_target3:
    /* Different safe operation */
    result = (code >> 1) ^ 0xAA;
    return result;
}

/* Test 6: Avoid using special registers (like $ra on MIPS) */
int test_no_special_regs(int a, int b) {
    volatile int prevent_opt = 0;
    
    if (prevent_opt || (a != b)) {
        COMPILER_BARRIER();
        goto safe_target;
    }
    
    return a + b;
    
safe_target:
    /* Only use parameters and local temps - no special registers */
    int tmp1 = a * 2;
    int tmp2 = b + 1;
    int result = tmp1 - tmp2;
    
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit condition creating a jump */
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto early_exit;
        }
    }
    
    return sum;
    
early_exit:
    /* Instruction at early exit target */
    sum = sum * 2;
    return sum;
}

/* Portable fallback for non-delay-slot architectures */
int test_portable_fallback(int a, int b) {
    /* Perform similar computations without relying on delay slot behavior */
    int result = a;
    
    if (a > b) {
        result = a + 1;
    } else {
        result = b - a;
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all test functions with various inputs */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(15, 5);
    checksum += test_multiple_instructions(7, 8, 1);
    checksum += test_nested_jumps(25);
    checksum += test_switch_jumps(5);
    checksum += test_no_special_regs(12, 18);
    checksum += test_loop_exit(50);
    
#if !HAS_DELAY_SLOTS
    /* Add portable version for architectures without delay slots */
    checksum += test_portable_fallback(10, 20);
#endif
    
    /* Additional iterations to increase optimization opportunities */
    for (i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i * 2);
        checksum += test_conditional_jump(i * 3, i);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify checksum is non-zero (sanity check) */
    if (checksum == 0) {
        printf("Warning: All computations optimized away!\n");
        return 1;
    }
    
    return 0;
}
