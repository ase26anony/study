/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__MIPS__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;
volatile int global_result = 0;

/* Memory barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

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
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is not live across the jump in this context */
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation at target */
int test_unconditional_jump_logical(int a, int b) {
    int temp = a;
    
    if (a != b) {
        COMPILER_BARRIER();
        /* Force unconditional jump pattern */
        goto target2;
    }
    
    return a & b;
    
target2:
    /* Candidate: bitwise operation with temporary variable */
    temp = temp ^ 0xFF;  /* Self-contained operation */
    return temp;
}

/* Test 3: Nested conditional with safe memory operation at target */
int test_jump_with_safe_memop(int *ptr, int val) {
    int local = val;
    int *safe_ptr = &local;  /* Use local address, not parameter */
    
    if (val > 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    return val * 2;
    
target3:
    /* Candidate: memory store to local variable (should be safe) */
    *safe_ptr = local + 5;
    return *safe_ptr;
}

/* Test 4: Multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int t1 = a, t2 = b, t3 = c;
    
    /* Complex condition to encourage jump generation */
    if ((a > b) && (b < c) && (a != c)) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return a + b + c;
    
target4:
    /* Multiple independent instructions - one might be eligible */
    t1 = t1 * 2;    /* Multiplication */
    t2 = t2 | 0x1;  /* Bitwise OR */
    t3 = t3 + t1;   /* Addition with previous result */
    
    return t3;
}

/* Test 5: Function with return jump pattern */
int test_return_jump_pattern(int x) {
    if (x == 0) {
        COMPILER_BARRIER();
        /* This might generate a jump to return sequence */
        goto early_return;
    }
    
    /* Normal path with computation */
    int y = x * x;
    return y + 1;
    
early_return:
    /* Simple instruction that doesn't use return address register */
    x = x & ~0x1;  /* Clear LSB */
    return x;
}

/* Test 6: Switch-like jump table pattern */
int test_switch_like_jump(int code) {
    int result = 0;
    
    switch(code & 0x3) {
        case 0:
            COMPILER_BARRIER();
            goto case0_target;
        case 1:
            return code + 1;
        case 2:
            return code * 2;
        case 3:
            return code / 2;
    }
    
    return result;
    
case0_target:
    /* Simple arithmetic at target */
    result = code + 100;
    return result;
}

/* Test 7: Loop exit jump pattern */
int test_loop_exit_jump(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            COMPILER_BARRIER();
            /* Jump out of loop */
            goto exit_loop;
        }
    }
    
    return sum;
    
exit_loop:
    /* Instruction at jump target */
    sum = sum >> 1;  /* Right shift */
    return sum;
}

/* Test 8: Minimal resource usage pattern */
int test_minimal_resources(int a) {
    register int r1 asm("$8") = a;  /* Suggest register on MIPS */
    register int r2 asm("$9") = 0;
    
    if (a & 0x1) {
        COMPILER_BARRIER();
        goto minimal_target;
    }
    
    return a;
    
minimal_target:
    /* Use only suggested registers to avoid conflicts */
    r2 = r1 + 5;
    return r2;
}

/* Architecture-specific inline assembly for direct control */
#if HAS_DELAY_SLOTS
/* Test with inline assembly to force specific instruction patterns */
int test_asm_direct_control(int a, int b) {
    int result;
    
    /* Force conditional branch assembly */
    __asm__ volatile (
        "move %0, %1\n\t"           /* result = a */
        "ble %1, %2, 1f\n\t"        /* if (a <= b) skip */
        "nop\n\t"                   /* Delay slot (might get filled) */
        "b 2f\n\t"                  /* Unconditional jump */
        "nop\n\t"                   /* Another delay slot */
        "1:\n\t"
        "move %0, %2\n\t"           /* result = b */
        "b 3f\n\t"
        "nop\n\t"
        "2:\n\t"
        /* Target label with candidate instruction */
        "addiu %0, %0, 1\n\t"       /* result = result + 1 */
        "3:\n\t"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "memory"
    );
    
    return result;
}
#endif

/* Main driver function */
int main() {
    int checksum = 0;
    int test_results[10];
    int i;
    
    /* Initialize test data */
    int test_a = 15;
    int test_b = 10;
    int test_c = 25;
    int test_array[5] = {50, 150, 200, 75, 125};
    
    printf("Delay Slot Test Program\n");
    printf("Architecture has delay slots: %s\n", HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests */
    test_results[0] = test_simple_jump_arithmetic(test_a, test_b);
    test_results[1] = test_unconditional_jump_logical(test_a, test_b);
    test_results[2] = test_jump_with_safe_memop(&test_array[0], test_c);
    test_results[3] = test_multiple_candidates(test_a, test_b, test_c);
    test_results[4] = test_return_jump_pattern(test_a);
    test_results[5] = test_switch_like_jump(test_c);
    test_results[6] = test_loop_exit_jump(50);
    test_results[7] = test_minimal_resources(test_b);
    
#if HAS_DELAY_SLOTS
    test_results[8] = test_asm_direct_control(test_a, test_b);
#else
    test_results[8] = test_a + test_b;  /* Fallback */
#endif
    
    /* Calculate checksum */
    for (i = 0; i < 9; i++) {
        checksum += test_results[i];
        printf("Test %d result: %d\n", i, test_results[i]);
    }
    
    /* Use results to prevent dead code elimination */
    global_result = checksum;
    
    printf("Total checksum: %d\n", checksum);
    printf("Global result: %d\n", global_result);
    
    /* Verify all tests executed */
    if (checksum != 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: All tests returned zero.\n");
        return 1;
    }
}
