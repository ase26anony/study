/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets lines 2135-2149 in reorg.cc
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

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    if (x > y) {
        COMPILER_BARRIER();
        /* This should generate a simple jump to label */
        goto target1;
    }
    
    /* Different path to avoid tail merging */
    result = y - x;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is not live across the jump in this context */
    return result;
}

/* Test 2: Unconditional jump created via condition */
int test_unconditional_pattern(int a, int b) {
    int temp = a;
    
    /* Force compiler to generate jump */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Dead code to avoid optimization */
    temp = b * 2;
    return temp;
    
target2:
    /* Multiple candidate instructions at target */
    temp = temp ^ 0xFF;  /* Bitwise operation - safe */
    temp = temp + global_a;  /* Use volatile to prevent optimization */
    return temp;
}

/* Test 3: Nested condition with safe target instruction */
int test_nested_condition(int val) {
    int local1 = val;
    int local2 = 0;
    
    if (val > 10) {
        if (val < 100) {
            COMPILER_BARRIER();
            goto target3;
        }
        local2 = val * 2;
    }
    
    return local1 + local2;
    
target3:
    /* Use completely fresh registers/temporaries */
    int fresh1 = local1;
    int fresh2 = global_b;
    fresh1 = fresh1 & fresh2;  /* Safe bitwise AND */
    return fresh1;
}

/* Test 4: Jump with multiple safe instructions at target */
int test_multiple_target_insts(int x) {
    int a = x;
    int b = 0;
    
    /* Create simple jump */
    if (a > 0) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    b = a * 3;
    return b;
    
target4:
    /* Sequence of safe instructions */
    int t1 = a;
    int t2 = global_c;
    t1 = t1 | 0x01;      /* OR operation */
    t2 = t2 ^ t1;        /* XOR with different register */
    return t1 + t2;
}

/* Test 5: Function with return jump pattern */
int test_return_jump(int x, int y) {
    int sum = x + y;
    
    if (sum > 50) {
        COMPILER_BARRIER();
        goto early_exit;
    }
    
    /* Complex enough to not be optimized into same block */
    for (int i = 0; i < 3; i++) {
        sum += i;
    }
    return sum;
    
early_exit:
    /* Safe instruction that doesn't use return address register */
    int modified = sum;
    modified = modified << 2;  /* Shift operation */
    return modified;
}

/* Test 6: Avoid using special registers (like $ra on MIPS) */
int test_safe_register_use(int *ptr, int n) {
    int val = *ptr;
    
    if (n > 0) {
        COMPILER_BARRIER();
        goto process;
    }
    
    val = -val;
    return val;
    
process:
    /* Only use argument registers or fresh temporaries */
    int temp = n;
    temp = temp + 1;  /* Simple increment */
    return temp;
}

/* Architecture-specific targeting */
#if HAS_DELAY_SLOTS
/* Test 7: Explicit inline assembly to guide instruction selection */
int test_asm_guided(int a, int b) {
    int res = a;
    
    /* Force conditional branch pattern */
    if (a > b) {
        /* Inline asm to prevent optimization */
        __asm__ volatile (
            ".set noreorder\n\t"
            ".set nomacro\n\t"
            : : : "memory"
        );
        goto asm_target;
    }
    
    res = b - a;
    return res;
    
asm_target:
    /* Instruction that should be safe for delay slot */
    res = res & 0x0F;  /* Mask operation */
    return res;
}

/* Test 8: Try to create jump to label with no resource conflicts */
int test_no_conflicts(int x) {
    register int r1 asm("$8") = x;  /* Suggest register $8 on MIPS */
    register int r2 asm("$9") = 0;  /* Suggest register $9 */
    
    if (r1 != 0) {
        COMPILER_BARRIER();
        goto no_conflict_target;
    }
    
    r2 = r1 * 2;
    return r2;
    
no_conflict_target:
    /* Use completely different register */
    register int r3 asm("$10") = 5;
    r3 = r3 + 1;
    return r3;
}
#endif

/* Driver function */
int main(void) {
    int checksum = 0;
    int results[8] = {0};
    
    /* Initialize with some values */
    int test_vals[] = {25, 10, 75, 30, 60, 5, 42, 18};
    
    /* Run all tests */
    results[0] = test_simple_jump_arithmetic(test_vals[0], test_vals[1]);
    results[1] = test_unconditional_pattern(test_vals[1], test_vals[2]);
    results[2] = test_nested_condition(test_vals[2]);
    results[3] = test_multiple_target_insts(test_vals[3]);
    results[4] = test_return_jump(test_vals[4], test_vals[5]);
    results[5] = test_safe_register_use(&global_a, test_vals[6]);
    
    #if HAS_DELAY_SLOTS
    results[6] = test_asm_guided(test_vals[6], test_vals[7]);
    results[7] = test_no_conflicts(test_vals[0]);
    #endif
    
    /* Calculate checksum to ensure all code executes */
    for (int i = 0; i < 8; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Test checksum: 0x%08X\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO (generic fallback)");
    
    return 0;
}
