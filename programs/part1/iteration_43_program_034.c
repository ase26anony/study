/* delay_slot_test.c - Test program for GCC delay slot filling optimization */

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
    result = x + 1;  /* Uses x which is already live, but safe */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Use distinct temporary variables to avoid resource conflicts */
    int t1 = temp1;
    int t2 = temp2;
    
    if (t1 > t2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    result = t1 - t2;
    return result;
    
target2:
    /* Candidate: Bitwise operation on temporaries */
    result = t1 & 0xFF;  /* Simple operation, no memory access */
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int result = 0;
    
    /* Use a register that's not live across the jump */
    int temp_reg = local1;
    
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    result = local2;
    return result;
    
target3:
    /* Multiple simple instructions - compiler may choose one for delay slot */
    temp_reg = temp_reg + 5;      /* Addition */
    temp_reg = temp_reg | 0x10;   /* Bitwise OR */
    result = temp_reg;
    return result;
}

/* Test 4: Nested jumps to create more complex patterns */
int test_nested_jump_pattern(int x, int y, int z) {
    int a = x;
    int b = y;
    int c = z;
    
    /* First level condition */
    if (a > b) {
        /* Second level condition */
        if (b < c) {
            COMPILER_BARRIER();
            goto target4;
        }
        return a - b;
    }
    
    return b - a;
    
target4:
    /* Safe instruction: shift operation */
    c = c << 2;  /* Simple shift, no side effects */
    return c;
}

/* Test 5: Function with switch that creates jump tables */
int test_switch_jump(int code) {
    int result = 0;
    int temp = code * 2;
    
    switch (code & 0x3) {  /* Mask to limit cases */
        case 0:
            COMPILER_BARRIER();
            goto target5_0;
        case 1:
            result = temp + 1;
            break;
        case 2:
            result = temp + 2;
            break;
        default:
            result = temp + 3;
            break;
    }
    return result;
    
target5_0:
    /* Simple arithmetic at target */
    result = temp ^ 0x55;  /* XOR operation */
    return result;
}

/* Test 6: Avoid using return address register conflicts */
int test_no_ra_conflict(int x) {
    int result = x;
    int safe_temp = x + 10;  /* Temporary not used before jump */
    
    /* Create condition that leads to jump */
    if (x % 2 == 0) {
        COMPILER_BARRIER();
        goto target6;
    }
    
    result = x * 3;
    return result;
    
target6:
    /* Use only the safe temporary to avoid conflicts */
    safe_temp = safe_temp + 1;
    result = safe_temp;
    return result;
}

/* Test 7: Multiple basic blocks with jumps between them */
int test_multiple_blocks(int a, int b, int c) {
    int r1 = a;
    int r2 = b;
    int r3 = c;
    
    if (r1 > r2) {
        if (r2 > r3) {
            COMPILER_BARRIER();
            goto block_a;
        } else {
            COMPILER_BARRIER();
            goto block_b;
        }
    }
    
    return r1 + r2 + r3;
    
block_a:
    r1 = r1 * 2;  /* Simple multiplication */
    return r1;
    
block_b:
    r2 = r2 | r3;  /* Bitwise OR */
    return r2;
}

/* Main driver that exercises all test patterns */
int main(void) {
    int checksum = 0;
    
    /* Run all tests with various inputs to explore different paths */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_unconditional_jump(0, 30);
    
    checksum += test_conditional_jump(50, 25);
    checksum += test_conditional_jump(25, 50);
    
    checksum += test_multiple_candidates(75);
    checksum += test_multiple_candidates(150);
    
    checksum += test_nested_jump_pattern(10, 5, 3);
    checksum += test_nested_jump_pattern(5, 10, 15);
    
    checksum += test_switch_jump(0);
    checksum += test_switch_jump(1);
    checksum += test_switch_jump(2);
    checksum += test_switch_jump(3);
    
    checksum += test_no_ra_conflict(8);
    checksum += test_no_ra_conflict(7);
    
    checksum += test_multiple_blocks(10, 5, 3);
    checksum += test_multiple_blocks(5, 10, 15);
    checksum += test_multiple_blocks(5, 3, 10);
    
    printf("Final checksum: %d\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "Yes (MIPS/SPARC)" : "No (generic)");
    
    /* Verify expected behavior */
    if (checksum != 0) {
        printf("Tests completed successfully.\n");
        return 0;
    } else {
        printf("Warning: All tests returned zero.\n");
        return 1;
    }
}
