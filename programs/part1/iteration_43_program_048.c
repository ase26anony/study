/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This code should not be reached when x != 0 */
    result = y * 2;
    return result;
    
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
    /* Candidate: Logical operation with temporaries */
    temp1 = temp1 & 0xFF;  /* Mask operation - safe */
    return temp1;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force a jump */
    if (val < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local1 = 0;
    return local1;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    local1 = local1 + 5;      /* Addition */
    local2 = local1 ^ 0x55;   /* XOR - no memory access */
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
    
    result = z;
    return result;
    
target4a:
    /* Simple shift operation */
    result = result << 2;
    return result;
    
target4b:
    /* Increment operation */
    result = result + 1;
    return result;
}

/* Test 5: Jump with register-only operations (no memory) */
int test_register_only(int a, int b) {
    register int r1 asm("t0") = a;  /* Suggest temporary register */
    register int r2 asm("t1") = b;
    
    if (r1 != r2) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    return r1 + r2;
    
target5:
    /* Pure register operation - good candidate for delay slot */
    r1 = r1 * r2;
    return r1;
}

/* Test 6: Avoid using special registers (like $ra on MIPS) */
int test_no_special_regs(int x) {
    int tmp = x;
    
    /* Don't use return address register in target instruction */
    if (tmp & 1) {  /* Check LSB */
        COMPILER_BARRIER();
        goto target6;
    }
    
    return tmp * 3;
    
target6:
    /* Safe: only uses local variable, no special registers */
    tmp = tmp | 0x100;
    return tmp;
}

/* Test 7: Multiple basic blocks with jumps */
int test_complex_flow(int a, int b, int c) {
    int val = a;
    
    if (a > b) {
        if (b > c) {
            COMPILER_BARRIER();
            goto label_a;
        } else {
            val = c;
            COMPILER_BARRIER();
            goto label_b;
        }
    } else if (a == b) {
        val = a + b;
        COMPILER_BARRIER();
        goto label_c;
    }
    
    val = 0;
    return val;
    
label_a:
    val = val + 10;
    return val;
    
label_b:
    val = val & 0x0F;
    return val;
    
label_c:
    val = val >> 1;
    return val;
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_assisted(void) {
    int a = global_a;
    int b = global_b;
    int result;
    
    /* Use inline asm to create a simple jump pattern */
    __asm__ volatile (
        "move %0, %1\n\t"          /* Copy a to result */
        "bnez %1, 1f\n\t"          /* Branch if a != 0 */
        "nop\n\t"                  /* Traditional delay slot */
        "move %0, %2\n\t"          /* Alternative: result = b */
        "b 2f\n\t"
        "nop\n"
        "1:\n\t"
        /* Target label - instruction here might be moved to delay slot */
        "addiu %0, %0, 1\n\t"      /* result = result + 1 */
        "2:\n\t"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "memory"
    );
    
    return result;
}
#endif

/* Main driver */
int main(void) {
    int checksum = 0;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all tests */
    checksum += test_unconditional_jump(5, 10);
    checksum += test_conditional_jump(global_a, global_b);
    checksum += test_multiple_candidates(50);
    checksum += test_nested_jumps(15, 10, 5);
    checksum += test_register_only(7, 3);
    checksum += test_no_special_regs(42);
    checksum += test_complex_flow(8, 6, 4);
    
#if HAS_DELAY_SLOTS
    checksum += test_asm_assisted();
    printf("Running architecture-specific delay slot tests\n");
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are non-zero */
    if (checksum == 0) {
        printf("ERROR: All tests returned zero!\n");
        return 1;
    }
    
    return 0;
}
