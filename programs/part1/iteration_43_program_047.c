/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
 */

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

/* Memory barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with safe arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Some code that won't be executed when jump is taken */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is not live across the jump */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Create conditional jump */
    if (a > b) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    temp1 = b - a;
    return temp1;
    
target2:
    /* Safe instruction: bitwise operation on temporaries */
    temp1 = temp1 ^ temp2;  /* Uses registers not live across jump */
    return temp1;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force a jump */
    if (val & 1) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local3 = val / 2;
    return local3;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    local1 = local1 + local2;    /* First candidate */
    local2 = local1 << 1;        /* Second candidate */
    return local1 + local2;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int result = x;
    
    if (x > y) {
        if (y > z) {
            COMPILER_BARRIER();
            goto target4a;
        } else {
            COMPILER_BARRIER();
            goto target4b;
        }
    }
    
    result = z - x;
    return result;
    
target4a:
    /* Safe arithmetic with constants */
    result = x * 3;
    return result;
    
target4b:
    /* Logical operation */
    result = y | z;
    return result;
}

/* Test 5: Jump that uses only local temporaries */
int test_local_temporaries(void) {
    int t1 = global_a;
    int t2 = global_b;
    int t3 = global_c;
    
    /* Create jump based on global comparison */
    if (global_a > global_b) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    t3 = t1 + t2;
    return t3;
    
target5:
    /* Instruction uses only local temporaries, no globals */
    t1 = t2 + 7;  /* Constant addition - very safe */
    return t1;
}

/* Test 6: Function with switch-like jump table pattern */
int test_switch_like(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            COMPILER_BARRIER();
            goto target6a;
        case 1:
            COMPILER_BARRIER();
            goto target6b;
        default:
            return code;
    }
    
target6a:
    result = code + 100;
    return result;
    
target6b:
    result = code * 2;
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto early_exit;
        }
    }
    return sum;
    
early_exit:
    /* Simple increment that's safe to move */
    sum = sum + 1;
    return sum;
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_delay_slot(void) {
    int a = global_a;
    int b = global_b;
    int result;
    
    /* Use inline asm to create specific instruction patterns */
    __asm__ volatile(
        "move %0, %1\n\t"          /* Start with a move */
        "bnez %1, 1f\n\t"          /* Conditional branch */
        "nop\n\t"                  /* Traditional nop in delay slot */
        "move %0, %2\n\t"
        "b 2f\n\t"
        "nop\n\t"
        "1:\n\t"
        "addu %0, %1, 1\n\t"       /* Candidate for delay slot filling */
        "2:\n\t"
        : "=r"(result)
        : "r"(a), "r"(b)
        : "memory"
    );
    
    return result;
}
#endif

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Delay Slot Test Program\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests and accumulate checksum */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(30, 15);
    checksum += test_multiple_candidates(25);
    checksum += test_nested_jumps(5, 10, 15);
    checksum += test_local_temporaries();
    checksum += test_switch_like(42);
    checksum += test_loop_exit(50);
    
#if HAS_DELAY_SLOTS
    checksum += test_asm_delay_slot();
    printf("Running architecture-specific delay slot tests\n");
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are reasonable */
    if (checksum != 0) {
        printf("Tests completed successfully\n");
        return 0;
    } else {
        printf("Warning: Checksum is zero\n");
        return 1;
    }
}
