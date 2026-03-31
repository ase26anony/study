/* test_delay_slots.c - Test program for GCC delay slot filling optimization */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_a = 42;
volatile int global_b = 17;

/* Barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Some code that won't be reached when x != 0 */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local temporaries not live across the jump */
    int temp1 = x + 1;      /* Should be safe to move */
    int temp2 = y - 1;
    result = temp1 + temp2;
    
    /* Additional code to prevent tail optimization */
    global_counter += result;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int local1 = a;
    int local2 = b;
    
    /* Different comparison to create conditional jump */
    if (local1 > local2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    local1 = local1 * 3;
    return local1;
    
target2:
    /* Candidate: bitwise operations (non-trapping) */
    /* Use fresh temporaries to avoid resource conflicts */
    int temp = local1 ^ local2;
    temp = temp & 0xFF;
    temp = temp | 0x100;
    
    global_counter += temp;
    return temp;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int x = val;
    int y = x * 2;
    
    if (x < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    y = y / 2;
    return y;
    
target3:
    /* Multiple simple instructions - one might be eligible */
    int t1 = x + 7;
    int t2 = t1 << 2;
    int t3 = t2 & 0x3F;
    
    /* Use all results to prevent dead code elimination */
    global_a = t1;
    global_b = t2;
    
    return t3;
}

/* Test 4: Nested jumps to create complex flow */
int test_nested_flow(int a, int b, int c) {
    int res = a;
    
    if (a > b) {
        if (b > c) {
            COMPILER_BARRIER();
            goto target4;
        }
        res = a + b;
    }
    
    res = res + c;
    return res;
    
target4:
    /* Safe arithmetic with constants only */
    int tmp = 5;
    tmp = tmp * 2;
    tmp = tmp + 1;
    
    global_counter += tmp;
    return tmp;
}

/* Test 5: Function with switch and goto */
int test_switch_jump(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            result = 1;
            break;
        case 1:
            COMPILER_BARRIER();
            goto target5;
        case 2:
            result = 3;
            break;
        default:
            return 4;
    }
    
    return result + 10;
    
target5:
    /* Very simple instruction - good candidate */
    int simple = 42;
    simple = simple + (code & 1);
    
    return simple;
}

/* Test 6: Loop with conditional exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < 10; i++) {
        sum += i;
        
        /* Early exit condition that creates a jump */
        if (i >= limit) {
            COMPILER_BARRIER();
            goto target6;
        }
    }
    
    return sum;
    
target6:
    /* Arithmetic with loop-invariant computation */
    int adjustment = limit * 2;
    adjustment = adjustment + 1;
    
    return sum + adjustment;
}

/* Test 7: Avoid using return address register conflicts */
int test_no_ra_conflict(int x) {
    int local = x;
    
    /* Create jump without function call context */
    if (local != 0) {
        COMPILER_BARRIER();
        goto target7;
    }
    
    return 0;
    
target7:
    /* Use only argument registers or temporaries */
    /* Avoid $ra on MIPS, %o7 on SPARC */
    int safe_op = local + 5;
    safe_op = safe_op * 2;
    
    return safe_op;
}

/* Architecture-specific targeting */
#if HAS_DELAY_SLOTS
/* Test 8: Explicit attempt with inline asm to guide RTL generation */
int test_asm_guided(void) {
    int a = 10, b = 20, c;
    
    /* Force a simple jump instruction */
    if (a < b) {
        /* Inline asm to create specific instruction pattern */
        __asm__ volatile(
            ".set noreorder\n\t"
            "b 1f\n\t"
            "nop\n\t"  /* Placeholder for potential delay slot fill */
            ".set reorder\n\t"
            : : : "memory"
        );
        
        goto asm_target;
    }
    
    return 0;
    
asm_target:
    /* Very simple instruction that should be delay-slot eligible */
    c = a + b;
    return c;
}
#endif

/* Main driver that exercises all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run test cases with various inputs */
    checksum += test_unconditional_jump(5, 3);
    checksum += test_conditional_jump(10, 5);
    checksum += test_conditional_jump(5, 10);
    checksum += test_multiple_candidates(50);
    checksum += test_multiple_candidates(150);
    checksum += test_nested_flow(10, 5, 2);
    checksum += test_nested_flow(5, 10, 2);
    checksum += test_switch_jump(0);
    checksum += test_switch_jump(1);
    checksum += test_switch_jump(2);
    checksum += test_loop_exit(3);
    checksum += test_loop_exit(15);
    checksum += test_no_ra_conflict(7);
    checksum += test_no_ra_conflict(0);
    
#if HAS_DELAY_SLOTS
    checksum += test_asm_guided();
#endif
    
    /* Additional iterations to increase optimization opportunities */
    for (i = 0; i < 100; i++) {
        checksum += test_unconditional_jump(i, i * 2);
        if (checksum & 1) {
            checksum += test_conditional_jump(i, checksum);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0) ? 0 : 1;
}
