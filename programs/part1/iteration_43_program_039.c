/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Barrier to prevent reordering across jumps */
#define JUMP_BARRIER() __asm__ volatile ("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    /* Create a simple conditional jump */
    if (x > y) {
        JUMP_BARRIER();
        goto target1;
    }
    
    /* Fall through path */
    result = y - x;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = x + 1;
    result = temp * 2;
    return result;
}

/* Test 2: Unconditional jump via goto with logical ops at target */
int test_unconditional_jump_logical(int x) {
    int result = x;
    
    if (x & 1) {
        JUMP_BARRIER();
        goto target2;
    }
    
    result = x >> 1;
    return result;
    
target2:
    /* Candidate: logical operations with temporary */
    int tmp = x;
    tmp = tmp ^ 0x5555;
    tmp = tmp & 0xFFFF;
    result = tmp;
    return result;
}

/* Test 3: Nested conditionals creating multiple jump opportunities */
int test_nested_jumps(int a, int b, int c) {
    int val = a;
    
    if (a > b) {
        if (b < c) {
            JUMP_BARRIER();
            goto target3;
        }
        val = a + b;
    } else {
        val = b - a;
    }
    
    return val;
    
target3:
    /* Multiple candidate instructions at target */
    int t1 = c;
    t1 = t1 + 7;
    t1 = t1 * 3;
    val = t1;
    return val;
}

/* Test 4: Jump based on external volatile to prevent constant folding */
int test_volatile_jump(void) {
    int a = global_a;
    int b = global_b;
    int result;
    
    if (a != b) {
        JUMP_BARRIER();
        goto target4;
    }
    
    result = a + b;
    return result;
    
target4:
    /* Safe arithmetic with no memory references */
    int tmp = a;
    tmp = tmp * 2;
    tmp = tmp + 5;
    result = tmp;
    return result;
}

/* Test 5: Function with multiple labels and jumps */
int test_multiple_labels(int x) {
    int r = x;
    
    if (x < 100) {
        JUMP_BARRIER();
        goto label_a;
    }
    
    if (x > 200) {
        JUMP_BARRIER();
        goto label_b;
    }
    
    r = x * 2;
    return r;
    
label_a:
    /* First candidate instruction set */
    r = x + 10;
    r = r | 1;
    return r;
    
label_b:
    /* Second candidate instruction set */
    r = x - 5;
    r = r & ~1;
    return r;
}

/* Test 6: Minimal function focusing on the specific pattern */
int test_minimal_pattern(int x) {
    /* Force a simple jump structure */
    if (x != 0) {
        /* Compiler barrier to prevent jump elimination */
        __asm__ volatile ("# jump barrier" : : : "memory");
        goto minimal_target;
    }
    
    return 0;
    
minimal_target:
    /* Single, simple instruction candidate */
    return x + 1;
}

/* Test 7: Loop with break to label (creates jump to exit) */
int test_loop_break_to_label(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            JUMP_BARRIER();
            goto loop_exit;
        }
    }
    
    return sum;
    
loop_exit:
    /* Arithmetic at exit label */
    sum = sum * 2;
    return sum;
}

/* Test 8: Switch-like construct using goto */
int test_goto_switch(int code) {
    int result = 0;
    
    if (code == 1) {
        JUMP_BARRIER();
        goto case1;
    } else if (code == 2) {
        JUMP_BARRIER();
        goto case2;
    }
    
    result = code;
    return result;
    
case1:
    result = code * 10;
    result = result + 5;
    return result;
    
case2:
    result = code * 20;
    result = result - 3;
    return result;
}

/* Portable fallback implementations for non-delay-slot architectures */
#if !HAS_DELAY_SLOTS
/* On non-delay-slot architectures, we still want to execute the logic */
int test_simple_jump_arithmetic(int x, int y) {
    return (x > y) ? (x + 1) * 2 : y - x;
}

int test_unconditional_jump_logical(int x) {
    return (x & 1) ? (x ^ 0x5555) & 0xFFFF : x >> 1;
}

int test_nested_jumps(int a, int b, int c) {
    return (a > b && b < c) ? (c + 7) * 3 : (a > b) ? a + b : b - a;
}

int test_volatile_jump(void) {
    int a = global_a;
    int b = global_b;
    return (a != b) ? a * 2 + 5 : a + b;
}

int test_multiple_labels(int x) {
    if (x < 100) return (x + 10) | 1;
    if (x > 200) return (x - 5) & ~1;
    return x * 2;
}

int test_minimal_pattern(int x) {
    return (x != 0) ? x + 1 : 0;
}

int test_loop_break_to_label(int limit) {
    int i, sum = 0;
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            sum *= 2;
            return sum;
        }
    }
    return sum;
}

int test_goto_switch(int code) {
    if (code == 1) return code * 10 + 5;
    if (code == 2) return code * 20 - 3;
    return code;
}
#endif

/* Main driver that exercises all test patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing delay slot patterns (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Test 1 */
    checksum += test_simple_jump_arithmetic(10, 5);
    checksum += test_simple_jump_arithmetic(5, 10);
    
    /* Test 2 */
    checksum += test_unconditional_jump_logical(7);
    checksum += test_unconditional_jump_logical(12);
    
    /* Test 3 */
    checksum += test_nested_jumps(10, 5, 8);
    checksum += test_nested_jumps(5, 10, 8);
    checksum += test_nested_jumps(10, 5, 3);
    
    /* Test 4 */
    checksum += test_volatile_jump();
    
    /* Test 5 */
    checksum += test_multiple_labels(50);
    checksum += test_multiple_labels(150);
    checksum += test_multiple_labels(250);
    
    /* Test 6 */
    checksum += test_minimal_pattern(42);
    checksum += test_minimal_pattern(0);
    
    /* Test 7 */
    checksum += test_loop_break_to_label(20);
    checksum += test_loop_break_to_label(100);
    
    /* Test 8 */
    checksum += test_goto_switch(1);
    checksum += test_goto_switch(2);
    checksum += test_goto_switch(3);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify against expected value for regression testing */
    int expected = 0;
#if HAS_DELAY_SLOTS
    /* Expected when delay slot code is active */
    expected = 10 > 5 ? (10 + 1) * 2 : 5 - 10;  /* 22 */
    expected += 5 > 10 ? (5 + 1) * 2 : 10 - 5;  /* 5 */
    expected += (7 & 1) ? (7 ^ 0x5555) & 0xFFFF : 7 >> 1;  /* 21862 */
    expected += (12 & 1) ? (12 ^ 0x5555) & 0xFFFF : 12 >> 1;  /* 6 */
    expected += (10 > 5 && 5 < 8) ? (8 + 7) * 3 : (10 > 5) ? 10 + 5 : 5 - 10;  /* 45 */
    expected += (5 > 10 && 10 < 8) ? (8 + 7) * 3 : (5 > 10) ? 5 + 10 : 10 - 5;  /* 5 */
    expected += (10 > 5 && 5 < 3) ? (3 + 7) * 3 : (10 > 5) ? 10 + 5 : 5 - 10;  /* 15 */
    expected += (42 != 17) ? 42 * 2 + 5 : 42 + 17;  /* 89 */
    expected += (50 < 100) ? (50 + 10) | 1 : (50 > 200) ? (50 - 5) & ~1 : 50 * 2;  /* 61 */
    expected += (150 < 100) ? (150 + 10) | 1 : (150 > 200) ? (150 - 5) & ~1 : 150 * 2;  /* 300 */
    expected += (250 < 100) ? (250 + 10) | 1 : (250 > 200) ? (250 - 5) & ~1 : 250 * 2;  /* 244 */
    expected += (42 != 0) ? 42 + 1 : 0;  /* 43 */
    expected += (0 != 0) ? 0 + 1 : 0;  /* 0 */
    /* Loop test - sum of 0..19 = 190, doesn't exceed 1000 */
    expected += 190;
    /* Loop test - sum exceeds 1000 at i=45, sum=1035, then *2 = 2070 */
    expected += 2070;
    expected += 1 * 10 + 5;  /* 15 */
    expected += 2 * 20 - 3;  /* 37 */
    expected += 3;  /* 3 */
#else
    /* Same calculation for portable version */
    expected = 22 + 5 + 21862 + 6 + 45 + 5 + 15 + 89 + 61 + 300 + 244 + 43 + 0 + 190 + 2070 + 15 + 37 + 3;
#endif
    
    if (checksum == expected) {
        printf("SUCCESS: Checksum matches expected value (%d)\n", expected);
        return 0;
    } else {
        printf("FAILURE: Checksum %d != expected %d\n", checksum, expected);
        return 1;
    }
}
