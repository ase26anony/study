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

/* Barrier to prevent unwanted optimizations */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Create a simple jump pattern */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = b * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot filling:
     * Simple arithmetic that doesn't conflict with jump resources
     * Uses local variable not live across the jump
     */
    result = result + 1;  /* Should be safe to move into delay slot */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int temp1 = x;
    int temp2 = y;
    
    /* Create conditional jump */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    temp1 = temp2 - 5;
    return temp1;
    
target2:
    /* Candidate: bitwise operation on fresh temporaries */
    temp1 = temp1 ^ 0xFF;  /* XOR operation - no memory access */
    return temp1;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force a jump */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local2 = local1 / 2;
    return local2;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 + 3;    /* First candidate */
    local1 = local1 & 0x0F; /* Second candidate - logical AND */
    return local1;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int a, int b, int c) {
    int t1 = a;
    int t2 = b;
    int t3 = c;
    
    if (t1 > t2) {
        if (t2 > t3) {
            COMPILER_BARRIER();
            goto target4a;
        } else {
            COMPILER_BARRIER();
            goto target4b;
        }
    }
    
    t1 = t2 + t3;
    return t1;
    
target4a:
    /* Simple increment - good delay slot candidate */
    t1 = t1 + 1;
    return t1;
    
target4b:
    /* Shift operation - also good candidate */
    t2 = t2 << 2;
    return t2;
}

/* Test 5: Avoid using special registers (like $ra on MIPS) */
int test_safe_registers(int x) {
    /* Use completely fresh variables to avoid resource conflicts */
    int fresh1 = x;
    int fresh2 = 0;
    
    /* Create jump */
    if (fresh1 != 0) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    fresh2 = 42;
    return fresh2;
    
target5:
    /* Use only the fresh variable that's defined before jump */
    fresh1 = fresh1 | 0x01;  /* Bitwise OR - safe operation */
    return fresh1;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int a) {
    int result = a;
    
    if (result > 10) {
        COMPILER_BARRIER();
        goto early_return;
    }
    
    result = result * 2;
    return result;
    
early_return:
    /* Simple arithmetic at return target */
    result = result - 1;
    return result;
}

/* Main driver that exercises all test patterns */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests with various inputs */
    checksum += test_unconditional_jump(5, 10);
    checksum += test_conditional_jump(20, 15);
    checksum += test_conditional_jump(5, 15);
    checksum += test_multiple_candidates(50);
    checksum += test_nested_jumps(10, 5, 3);
    checksum += test_nested_jumps(5, 10, 3);
    checksum += test_safe_registers(7);
    checksum += test_safe_registers(0);
    checksum += test_return_jump(15);
    checksum += test_return_jump(5);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify expected results */
    int expected = 0;
    
    /* Calculate expected based on portable semantics */
    expected += 5 + 1;                    /* test_unconditional_jump(5,10) */
    expected += (20 ^ 0xFF);              /* test_conditional_jump(20,15) */
    expected += 15 - 5;                   /* test_conditional_jump(5,15) */
    expected += ((50 + 3) & 0x0F);        /* test_multiple_candidates(50) */
    expected += 10 + 1;                   /* test_nested_jumps(10,5,3) */
    expected += (5 << 2);                 /* test_nested_jumps(5,10,3) */
    expected += (7 | 0x01);               /* test_safe_registers(7) */
    expected += 42;                       /* test_safe_registers(0) */
    expected += 15 - 1;                   /* test_return_jump(15) */
    expected += 5 * 2;                    /* test_return_jump(5) */
    
    printf("Expected checksum: %d\n", expected);
    
    if (checksum == expected) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Test mismatch! (This might be OK if delay slots were filled)\n");
        return 1;
    }
}
