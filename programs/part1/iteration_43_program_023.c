/* delay_slot_test.c - Test program for GCC delay slot filling optimization */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

/* Memory barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    if (x > y) {
        COMPILER_BARRIER();
        /* This should generate a simple jump to label */
        goto target1;
    }
    
    /* Fall through path */
    result = y - x;
    return result;
    
target1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = x + 1;
    result = temp * 2;
    return result;
}

/* Test 2: Unconditional jump via goto with logical ops at target */
int test_unconditional_jump_logical(int a, int b) {
    int val = a;
    
    if (a != b) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    return a | b;
    
target2:
    /* Candidate: bitwise operations, no memory access */
    val = val ^ 0xFF;
    val = val & 0x7F;
    return val;
}

/* Test 3: Nested conditional with safe computation at target */
int test_nested_conditional(int x) {
    int tmp = x;
    
    if (x > 10) {
        if (x < 100) {
            COMPILER_BARRIER();
            goto target3;
        }
        return x * 2;
    }
    
    return x / 2;
    
target3:
    /* Safe arithmetic with constants only */
    tmp = tmp + 5;
    tmp = tmp * 3;
    tmp = tmp - 15;
    return tmp;
}

/* Test 4: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int res = a;
    
    /* Complex condition to force jump generation */
    if ((a > b) && (b < c) && (a != c)) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return a + b + c;
    
target4:
    /* Multiple simple instructions that could fill delay slots */
    int t1 = b << 2;
    int t2 = c >> 1;
    res = t1 + t2;
    return res;
}

/* Test 5: Function with return jump pattern */
int test_return_jump_pattern(int x) {
    if (x == 0) {
        COMPILER_BARRIER();
        goto early_exit;
    }
    
    /* Some computation */
    int y = x * x;
    return y % 100;
    
early_exit:
    /* Instruction that doesn't use return address register */
    int z = x + 1;
    return z;
}

/* Test 6: Switch-like pattern with goto */
int test_switch_like(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            result = code + 1;
            break;
        case 1:
            COMPILER_BARRIER();
            goto case_target;
        case 2:
            result = code * 2;
            break;
        default:
            result = code - 1;
            break;
    }
    
    return result;
    
case_target:
    /* Simple arithmetic at target */
    result = code + 100;
    result = result & 0x7F;
    return result;
}

/* Test 7: Loop with conditional exit jump */
int test_loop_exit_jump(int limit) {
    int sum = 0;
    int i;
    
    for (i = 0; i < limit; i++) {
        if (i == limit / 2) {
            COMPILER_BARRIER();
            goto loop_exit;
        }
        sum += i;
    }
    
    return sum;
    
loop_exit:
    /* Safe computation at exit point */
    sum = sum * 2;
    return sum;
}

/* Test 8: Pointer arithmetic at target (safe if no dereference) */
int test_pointer_arithmetic(int *ptr, int idx) {
    int *p = ptr;
    
    if (idx > 0) {
        COMPILER_BARRIER();
        goto ptr_target;
    }
    
    return *ptr;
    
ptr_target:
    /* Pointer arithmetic without dereference */
    p = p + idx;
    return (int)(p - ptr);
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_guided(void) {
    int a = global_a;
    int b = global_b;
    int result;
    
    /* Force a simple jump instruction */
    if (a > b) {
        /* Inline asm to influence code generation */
        __asm__ volatile (
            "nop \n\t"
            : : : "memory"
        );
        
        goto asm_target;
    }
    
    result = a + b;
    return result;
    
asm_target:
    /* Instruction designed to be delay-slot eligible */
    result = a - b;
    /* Another asm to prevent optimization */
    __asm__ volatile (
        "nop \n\t"
        : : : "memory"
    );
    return result;
}
#endif

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int results[10];
    int i;
    
    /* Initialize with some values */
    int test_vals[] = {5, 10, 15, 20, 25, 30, 35, 40};
    
    /* Run all test functions */
    results[0] = test_simple_jump_arithmetic(test_vals[0], test_vals[1]);
    results[1] = test_unconditional_jump_logical(test_vals[1], test_vals[2]);
    results[2] = test_nested_conditional(test_vals[2]);
    results[3] = test_multiple_candidates(test_vals[3], test_vals[4], test_vals[5]);
    results[4] = test_return_jump_pattern(test_vals[4]);
    results[5] = test_switch_like(test_vals[5]);
    results[6] = test_loop_exit_jump(test_vals[6]);
    
    int dummy_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    results[7] = test_pointer_arithmetic(dummy_array, test_vals[7]);
    
    #if HAS_DELAY_SLOTS
    results[8] = test_asm_guided();
    printf("Compiled for architecture with delay slots (MIPS/SPARC)\n");
    #else
    results[8] = 0;
    printf("Compiled for architecture without delay slots\n");
    #endif
    
    /* Calculate checksum to ensure all computations are used */
    for (i = 0; i < 9; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Test results checksum: 0x%08X\n", checksum);
    
    /* Verify some results to prevent dead code elimination */
    if (checksum == 0) {
        printf("Warning: All results were zero!\n");
    }
    
    return 0;
}
