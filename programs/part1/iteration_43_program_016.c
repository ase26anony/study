/* Test program to trigger delay slot filling from jump target in GCC's reorg pass */
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

/* Test 1: Simple unconditional jump to label with safe arithmetic */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Force a simple jump structure */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken with typical test inputs */
    return b * 2;
    
target1:
    /* Candidate instruction for delay slot:
       - Simple arithmetic
       - Uses local variable not live across the jump
       - No memory references
       - No function calls
       - Not a jump */
    result = result + 1;  /* Should be eligible for delay slot */
    
    /* Additional code to prevent tail optimization */
    COMPILER_BARRIER();
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int temp = x;
    
    /* Create a conditional jump */
    if (x > y) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    temp = y - x;
    return temp;
    
target2:
    /* Candidate: bitwise operation on local variable */
    temp = temp | 0x01;  /* Simple, non-trapping operation */
    
    COMPILER_BARRIER();
    return temp;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int a = val;
    int b = val * 2;
    
    if (val % 2 == 0) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    return val + 100;
    
target3:
    /* Multiple simple instructions - compiler may choose one for delay slot */
    a = a + 5;
    b = b - 3;
    
    /* Use both to prevent dead code elimination */
    COMPILER_BARRIER();
    return a + b;
}

/* Test 4: Nested jumps to create more complex patterns */
int test_nested_jumps(int p, int q) {
    int res = p;
    
    if (p > q) {
        if (p - q > 10) {
            COMPILER_BARRIER();
            goto target4;
        }
        return p + q;
    }
    
    return q - p;
    
target4:
    /* Very simple instruction - good candidate */
    res = res & ~0x01;  /* Clear lowest bit */
    
    COMPILER_BARRIER();
    return res;
}

/* Test 5: Jump with register-only operations (avoid memory) */
int test_register_only(int r1, int r2) {
    int t1 = r1;
    int t2 = r2;
    
    /* Force jump with COMPILER_BARRIER */
    if (t1 != t2) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    return t1;
    
target5:
    /* Register-to-register operation - less likely to have resource conflicts */
    t1 = t1 ^ t2;  /* XOR operation */
    
    COMPILER_BARRIER();
    return t1;
}

/* Test 6: Avoid using special registers (like $ra on MIPS) */
int test_no_special_regs(int a, int b, int c) {
    /* Use only caller-saved/temporary registers in the pattern */
    int tmp1 = a;
    int tmp2 = b;
    int tmp3 = c;
    
    if (tmp1 + tmp2 > tmp3) {
        COMPILER_BARRIER();
        goto target6;
    }
    
    return tmp3;
    
target6:
    /* Simple arithmetic on temporaries */
    tmp1 = tmp1 * 2;
    tmp2 = tmp2 + 1;
    
    COMPILER_BARRIER();
    return tmp1 + tmp2;
}

/* Main driver that calls all tests */
int main(void) {
    int checksum = 0;
    
    /* Run tests with various inputs to explore different paths */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(15, 5);
    checksum += test_conditional_jump(5, 15);  /* Different branch direction */
    checksum += test_multiple_candidates(7);
    checksum += test_multiple_candidates(8);   /* Even number */
    checksum += test_nested_jumps(30, 15);
    checksum += test_nested_jumps(5, 3);       /* Different path */
    checksum += test_register_only(42, 42);    /* Equal case */
    checksum += test_register_only(42, 24);    /* Not equal case */
    checksum += test_no_special_regs(1, 2, 3);
    checksum += test_no_special_regs(10, 20, 5);
    
    printf("Result checksum: %d\n", checksum);
    
    /* Architecture-specific message */
#if HAS_DELAY_SLOTS
    printf("Compiled for architecture with delay slots\n");
#else
    printf("Compiled for architecture without delay slots (generic fallback)\n");
#endif
    
    return checksum != 0 ? 0 : 1;
}
