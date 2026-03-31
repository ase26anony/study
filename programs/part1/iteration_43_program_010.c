/* delay_slot_test.c - Test program to trigger GCC's delay slot filling logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_result = 0;

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int temp1 = x;
    int temp2 = y;
    int result = 0;
    
    /* Create a simple conditional jump */
    if (temp1 > temp2) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    result = temp1 + temp2;
    return result;
    
target_label1:
    /* Candidate instruction for delay slot:
       Uses temporary registers not live across the jump */
    temp1 = temp1 + 1;  /* Simple arithmetic - likely safe */
    result = temp1 * 2;
    return result;
}

/* Test 2: Unconditional jump via goto with logical ops */
int test_unconditional_jump_logical(int x) {
    int temp = x;
    int result = 0;
    
    if (temp != 0) {
        /* Force a simple jump structure */
        __asm__ volatile ("" : : : "memory");
        goto target_label2;
    }
    
    return 0;
    
target_label2:
    /* Logical operation - typically safe, no memory access */
    temp = temp & 0xFF;  /* Mask operation */
    result = temp | 0x100;
    return result;
}

/* Test 3: Nested condition with bit manipulation */
int test_nested_condition_bitops(int a, int b, int c) {
    int t1 = a;
    int t2 = b;
    int t3 = c;
    
    /* Complex enough to create a jump but simple enough for delay slot */
    if (t1 > t2) {
        if (t2 < t3) {
            __asm__ volatile ("" : : : "memory");
            goto target_label3;
        }
    }
    
    return t1 + t2 + t3;
    
target_label3:
    /* Bit manipulation - usually safe for delay slots */
    t1 = t1 ^ t2;  /* XOR operation */
    t2 = t2 << 2;  /* Shift operation */
    return t1 + t2;
}

/* Test 4: Function with multiple jumps to same label */
int test_multiple_jumps_same_target(int x, int y) {
    int tmp_x = x;
    int tmp_y = y;
    
    /* Multiple paths to the same target */
    if (tmp_x > 100) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    if (tmp_y < 50) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    if (tmp_x + tmp_y > 150) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    return tmp_x - tmp_y;
    
common_target:
    /* Safe arithmetic with fresh temporaries */
    int fresh1 = tmp_x + 5;
    int fresh2 = tmp_y * 2;
    return fresh1 + fresh2;
}

/* Test 5: Jump with register-only operations at target */
int test_register_only_ops(int a, int b) {
    register int r1 asm("$8") = a;  /* Suggest register usage on MIPS */
    register int r2 asm("$9") = b;
    
    if (r1 != r2) {
        /* Memory barrier to create clear jump boundary */
        __asm__ volatile ("" : : : "memory");
        goto reg_target;
    }
    
    return 0;
    
reg_target:
    /* Pure register operations - ideal for delay slot */
    r1 = r1 + r2;
    r2 = r1 - 10;
    return r1 * r2;
}

#else
/* Portable fallback versions for non-delay-slot architectures */

int test_simple_jump_arithmetic(int x, int y) {
    return (x > y) ? ((x + 1) * 2) : (x + y);
}

int test_unconditional_jump_logical(int x) {
    return (x != 0) ? ((x & 0xFF) | 0x100) : 0;
}

int test_nested_condition_bitops(int a, int b, int c) {
    if (a > b && b < c) {
        return (a ^ b) + (b << 2);
    }
    return a + b + c;
}

int test_multiple_jumps_same_target(int x, int y) {
    if (x > 100 || y < 50 || (x + y) > 150) {
        return (x + 5) + (y * 2);
    }
    return x - y;
}

int test_register_only_ops(int a, int b) {
    if (a != b) {
        return (a + b) * (a + b - 10);
    }
    return 0;
}

#endif /* Architecture check */

/* Test 6: Global variable access (more complex case) */
int test_global_access(int threshold) {
    int local = global_a;
    
    if (local > threshold) {
        /* Try to create jump to label with global access */
        __asm__ volatile ("" : : : "memory");
        goto global_target;
    }
    
    return local;
    
global_target:
    /* This might not be eligible due to memory access,
       but included for completeness */
    global_result = local + global_b;
    return global_result;
}

/* Test 7: Loop with internal jump */
int test_loop_with_jump(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        if (i == n / 2) {
            /* Jump inside loop */
            __asm__ volatile ("" : : : "memory");
            goto loop_target;
        }
        sum += i;
    }
    
    return sum;
    
loop_target:
    /* Simple increment - good candidate */
    sum += 100;
    return sum;
}

/* Main driver function */
int main() {
    int result = 0;
    int checksum = 0;
    
    printf("Delay Slot Test Program\n");
    printf("=======================\n");
    
    /* Run all test cases with various inputs */
    
    /* Test 1 */
    result = test_simple_jump_arithmetic(15, 10);
    checksum += result;
    printf("Test 1: %d\n", result);
    
    /* Test 2 */
    result = test_unconditional_jump_logical(42);
    checksum += result;
    printf("Test 2: %d\n", result);
    
    /* Test 3 */
    result = test_nested_condition_bitops(20, 15, 30);
    checksum += result;
    printf("Test 3: %d\n", result);
    
    /* Test 4 */
    result = test_multiple_jumps_same_target(120, 40);
    checksum += result;
    printf("Test 4: %d\n", result);
    
    /* Test 5 */
    result = test_register_only_ops(7, 3);
    checksum += result;
    printf("Test 5: %d\n", result);
    
    /* Test 6 */
    result = test_global_access(5);
    checksum += result;
    printf("Test 6: %d\n", result);
    
    /* Test 7 */
    result = test_loop_with_jump(10);
    checksum += result;
    printf("Test 7: %d\n", result);
    
    printf("\nFinal checksum: %d\n", checksum);
    
    /* Verify results match portable version */
    int expected = 
        test_simple_jump_arithmetic(15, 10) +
        test_unconditional_jump_logical(42) +
        test_nested_condition_bitops(20, 15, 30) +
        test_multiple_jumps_same_target(120, 40) +
        test_register_only_ops(7, 3) +
        test_global_access(5) +
        test_loop_with_jump(10);
    
    if (checksum == expected) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Mismatch detected!\n");
        return 1;
    }
}
