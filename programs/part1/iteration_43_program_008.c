/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    /* Create a simple conditional jump */
    if (x > y) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target1;
    }
    
    /* Fall-through path */
    result = y - x;
    return result;
    
target1:
    /* Candidate instruction for delay slot filling:
     * Simple arithmetic that doesn't conflict with jump resources
     */
    result = x + 1;  /* Should be safe to move into delay slot */
    
    /* More code to prevent tail optimization */
    result += global_a;
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation */
int test_unconditional_jump_logical(int x) {
    int temp = x;
    
    /* Force an unconditional jump pattern */
    if (temp != 0) {
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    return 0;
    
target2:
    /* Candidate: Bitwise operation with temporary register */
    temp = temp & 0xFF;  /* Should not conflict with jump resources */
    
    /* Use result to prevent dead code elimination */
    return temp | global_b;
}

/* Test 3: Nested conditional with safe memory operation */
int test_conditional_with_safe_load(int *ptr, int idx) {
    int local_temp;
    int result = 0;
    
    /* Create conditional jump */
    if (ptr != NULL && idx > 0) {
        /* Use volatile asm to create barrier */
        __asm__ volatile ("" : : : "memory");
        goto target3;
    }
    
    /* Alternative path */
    if (ptr) result = ptr[0];
    return result;
    
target3:
    /* Safe operation: use local variable only */
    local_temp = idx * 2;  /* Multiplication - should be safe */
    
    /* Access memory through pointer (could be unsafe, but ptr is non-null here) */
    if (ptr && idx == 1) {
        result = ptr[0] + local_temp;
    }
    
    return result;
}

/* Test 4: Multiple jumps to same label with different conditions */
int test_multi_path_jump(int a, int b, int c) {
    int temp = a;
    
    /* Multiple conditions leading to same label */
    if (a > b) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    if (b > c) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    if (c > a) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    /* Fall through */
    return a + b + c;
    
common_target:
    /* Simple instruction that should be delay-slot eligible */
    temp = temp ^ b;  /* XOR operation - typically safe */
    
    return temp + c;
}

/* Test 5: Function with return jump pattern */
int test_return_jump_pattern(int x) {
    int local1 = x;
    int local2 = x * 2;
    
    /* Pattern that might create a simplejump_p */
    if (local1 > 100) {
        /* Force jump to label before return */
        __asm__ volatile ("" : : : "memory");
        goto pre_return;
    }
    
    local2 = local1 + 50;
    return local2;
    
pre_return:
    /* Instruction that doesn't use special registers */
    local1 = local1 - 10;
    
    return local1;
}

/* Test 6: Loop with break to label */
int test_loop_break_jump(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Conditional break to label */
        if (sum > 1000) {
            __asm__ volatile ("" : : : "memory");
            goto loop_exit;
        }
    }
    
    return sum;
    
loop_exit:
    /* Simple arithmetic at target */
    sum = sum >> 1;  /* Right shift - should be safe */
    
    return sum;
}

/* Test 7: Switch with default goto */
int test_switch_goto(int val) {
    int result = val;
    
    switch (val & 3) {
        case 0:
            result += 10;
            break;
        case 1:
            result += 20;
            break;
        default:
            /* Jump to label from default case */
            __asm__ volatile ("" : : : "memory");
            goto switch_target;
    }
    
    return result;
    
switch_target:
    /* Safe instruction at target */
    result = result & ~1;  /* Clear LSB */
    
    return result + global_c;
}

/* Driver function that calls all tests */
int main(void) {
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int checksum = 0;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests with various inputs */
    checksum += test_simple_jump_arithmetic(global_a, global_b);
    checksum += test_unconditional_jump_logical(global_c);
    checksum += test_conditional_with_safe_load(test_array, 3);
    checksum += test_multi_path_jump(10, 20, 30);
    checksum += test_return_jump_pattern(150);
    checksum += test_loop_break_jump(50);
    checksum += test_switch_goto(7);
    
    /* Add architecture-specific tests if supported */
#if HAS_DELAY_SLOTS
    /* Additional tests that might better trigger delay slot filling */
    checksum += test_simple_jump_arithmetic(100, 50);
    checksum += test_multi_path_jump(5, 15, 25);
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are non-zero */
    if (checksum == 0) {
        printf("WARNING: All tests returned zero!\n");
        return 1;
    }
    
    return 0;
}
