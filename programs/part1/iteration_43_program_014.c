/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
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
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_result = 0;

/* Barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
int test_conditional_jump_arithmetic(int x, int y) {
    int result = x;
    
    /* Create a simple conditional jump */
    if (x > y) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Fall-through path */
    result = y - x;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = x + 1;
    result = temp * 2;
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation */
int test_unconditional_jump_logical(int a, int b) {
    int val = a;
    
    /* Force an unconditional jump pattern */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* This path should not be taken */
    return b;
    
target2:
    /* Candidate: logical operation with independent register */
    int mask = 0xFF;
    val = val & mask;
    return val | 0x100;
}

/* Test 3: Nested condition with bit manipulation */
int test_nested_condition_bitops(int x) {
    int result = x;
    
    if (x > 10) {
        if (x < 100) {
            COMPILER_BARRIER();
            goto target3;
        }
    }
    
    result = x >> 1;
    return result;
    
target3:
    /* Candidate: bit shift operation */
    result = (x << 2) | 1;
    return result;
}

/* Test 4: Jump based on global variable */
int test_global_based_jump(void) {
    int local = global_a;
    
    if (global_a > global_b) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return global_b;
    
target4:
    /* Candidate: arithmetic with constants */
    local = local + 7;
    local = local * 3;
    return local;
}

/* Test 5: Multiple candidate instructions at target */
int test_multiple_candidates(int a, int b) {
    int res = a;
    
    /* Create multiple basic blocks to encourage jump optimization */
    if (a == b) {
        return a + b;
    } else if (a > b) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    res = b - a;
    return res;
    
target5:
    /* Multiple instructions that could be candidates */
    int t1 = a + b;
    int t2 = t1 * 2;
    res = t2 - 1;
    
    /* Additional independent operation */
    int dummy = b & 0xF;
    (void)dummy; /* Prevent unused warning */
    
    return res;
}

/* Test 6: Function with early return jump */
int test_early_return(int x) {
    if (x <= 0) {
        COMPILER_BARRIER();
        goto early_exit;
    }
    
    /* Some computation */
    int y = x * x;
    
    if (y > 1000) {
        return y;
    }
    
early_exit:
    /* Candidate: simple increment */
    x = x + 1;
    return x;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int sum = 0;
    int i;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit condition that creates a jump */
        if (sum > 100) {
            COMPILER_BARRIER();
            goto exit_loop;
        }
    }
    
    return sum;
    
exit_loop:
    /* Candidate: final computation */
    sum = sum * 2;
    return sum;
}

/* Test 8: Switch statement with default jump */
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
            COMPILER_BARRIER();
            goto default_case;
    }
    
    return result;
    
default_case:
    /* Candidate: arithmetic sequence */
    result = code + 5;
    result = result * result;
    return result;
}

/* Architecture-specific test that uses inline assembly for precise control */
#if HAS_DELAY_SLOTS
int test_asm_controlled_jump(int a, int b) {
    int result;
    
    /* Use inline assembly to create a clean jump pattern */
    __asm__ volatile (
        "move %0, %1\n\t"           /* result = a */
        "ble %1, %2, 1f\n\t"        /* if (a <= b) skip */
        "nop\n\t"                   /* Traditional delay slot */
        "b 2f\n\t"                  /* Jump to target */
        "nop\n\t"                   /* Delay slot */
        "1:\n\t"
        "sub %0, %2, %1\n\t"        /* result = b - a */
        "b 3f\n\t"
        "nop\n\t"
        "2:\n\t"
        /* Candidate instruction for delay slot filling */
        "addiu %0, %1, 1\n\t"       /* result = a + 1 */
        "sll %0, %0, 1\n\t"         /* result = result * 2 */
        "3:\n\t"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "memory"
    );
    
    return result;
}
#else
/* Portable version for non-delay-slot architectures */
int test_asm_controlled_jump(int a, int b) {
    return (a > b) ? ((a + 1) * 2) : (b - a);
}
#endif

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    /* Seed with some values */
    int test_values[] = {5, 15, 25, 35, 45, 55, 65, 75};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Running delay slot filling tests...\n");
    printf("Architecture has delay slots: %s\n", HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run each test multiple times with different values */
    for (i = 0; i < num_tests; i++) {
        int val = test_values[i];
        
        checksum += test_conditional_jump_arithmetic(val, 20);
        checksum += test_unconditional_jump_logical(val, 30);
        checksum += test_nested_condition_bitops(val);
        checksum += test_global_based_jump();
        checksum += test_multiple_candidates(val, 25);
        checksum += test_early_return(val);
        checksum += test_loop_exit(val);
        checksum += test_switch_jump(val % 4);
        checksum += test_asm_controlled_jump(val, 20);
        
        /* Update globals to vary conditions */
        global_a = val;
        global_b = val / 2 + 5;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global result: %d\n", global_result);
    
    /* Verify checksum is non-zero (tests executed) */
    if (checksum == 0) {
        printf("WARNING: All tests returned zero!\n");
        return 1;
    }
    
    return 0;
}
