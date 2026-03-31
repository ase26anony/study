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

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    /* Create a simple conditional jump */
    if (x > y) {
        /* Force a simple jump to label */
        __asm__ volatile ("" : : : "memory");  /* Compiler barrier */
        goto target_label_1;
    }
    
    /* Alternative path */
    result = y - x;
    return result;
    
target_label_1:
    /* Candidate instruction for delay slot filling */
    /* Uses temporary variable not live across the jump */
    int temp = x + 1;  /* Simple arithmetic - should be safe */
    result = temp * 2;
    
    /* Prevent tail call optimization */
    global_counter += result;
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation */
int test_unconditional_jump_logical(int a, int b) {
    int val = a;
    
    /* Always jump */
    if (a != 0) {
        __asm__ volatile ("" : : : "memory");
        goto target_label_2;
    }
    
    /* This path should never be taken */
    return b;
    
target_label_2:
    /* Logical operation - unlikely to trap */
    val = val & 0xFFFF;  /* Mask operation */
    val = val | 0x1000;
    
    global_counter += val;
    return val;
}

/* Test 3: Nested conditional with bit manipulation */
int test_nested_conditional_bits(int x) {
    int temp1 = x;
    int temp2 = 0;
    
    /* Complex condition to create jump */
    if (x > 10 && x < 100) {
        if (x % 3 == 0) {
            __asm__ volatile ("" : : : "memory");
            goto target_label_3;
        }
        temp2 = x * 2;
    }
    
    return temp2;
    
target_label_3:
    /* Bit manipulation - safe operations */
    temp1 = temp1 << 2;    /* Shift left */
    temp1 = temp1 ^ 0xAA;  /* XOR with constant */
    
    global_counter += temp1;
    return temp1;
}

/* Test 4: Jump based on global variable comparison */
int test_global_based_jump(void) {
    int local = global_a;
    
    /* Compare globals to force jump */
    if (global_a > global_b) {
        __asm__ volatile ("" : : : "memory");
        goto target_label_4;
    }
    
    local = global_b - global_a;
    return local;
    
target_label_4:
    /* Safe arithmetic with local variable */
    local = local + global_counter;
    local = local * 2;
    
    global_a = local;  /* Store back to global */
    return local;
}

/* Test 5: Multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int result = a;
    
    /* Multiple conditions to create jump opportunity */
    if (a > b && b < c) {
        __asm__ volatile ("" : : : "memory");
        goto target_label_5;
    }
    
    result = c - b;
    return result;
    
target_label_5:
    /* Sequence of simple instructions */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 >> 1;      /* Shift instead of division */
    result = t3 & 0xFF;    /* Mask to byte */
    
    /* Use result to prevent dead code elimination */
    global_counter = (global_counter + result) & 0xFFFFFF;
    return result;
}

/* Test 6: Function with early return jump */
int test_early_return_jump(int x) {
    /* Early return creates jump to function exit */
    if (x < 0) {
        __asm__ volatile ("" : : : "memory");
        goto early_exit;
    }
    
    /* Normal processing */
    int y = x * x;
    y = y + x;
    
    global_counter += y;
    return y;
    
early_exit:
    /* Instruction at jump target before return */
    int neg = -x;
    neg = neg + 1;
    
    /* Force a store to prevent optimization */
    global_b = neg;
    return neg;
}

/* Test 7: Loop with conditional break jump */
int test_loop_break_jump(int limit) {
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        sum += i;
        
        /* Conditional break that creates a jump */
        if (i >= limit && limit > 0) {
            __asm__ volatile ("" : : : "memory");
            goto loop_exit;
        }
    }
    
    return sum;
    
loop_exit:
    /* Post-loop processing at jump target */
    sum = sum * 2;
    sum = sum + limit;
    
    global_counter += sum;
    return sum;
}

/* Test 8: Switch statement with default jump */
int test_switch_default_jump(int code) {
    int value = 0;
    
    switch (code) {
        case 1:
            value = 10;
            break;
        case 2:
            value = 20;
            break;
        case 3:
            value = 30;
            break;
        default:
            /* Jump to handle default case */
            __asm__ volatile ("" : : : "memory");
            goto default_handler;
    }
    
    return value;
    
default_handler:
    /* Default case handling */
    value = code * 5;
    value = value & 0x3F;  /* Safe operation */
    
    global_counter = (global_counter + value) % 1000;
    return value;
}

/* Portable fallback implementations */
#if !HAS_DELAY_SLOTS
/* For non-delay-slot architectures, ensure code still works */
int test_simple_jump_arithmetic_portable(int x, int y) {
    return (x > y) ? ((x + 1) * 2) : (y - x);
}

int test_unconditional_jump_logical_portable(int a, int b) {
    return (a | 0x1000) & 0xFFFF;
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
#if HAS_DELAY_SLOTS
    /* Run all delay-slot specific tests */
    checksum += test_simple_jump_arithmetic(10, 5);
    checksum += test_simple_jump_arithmetic(5, 10);
    
    checksum += test_unconditional_jump_logical(100, 200);
    checksum += test_unconditional_jump_logical(0, 300);
    
    checksum += test_nested_conditional_bits(30);
    checksum += test_nested_conditional_bits(15);
    checksum += test_nested_conditional_bits(5);
    
    checksum += test_global_based_jump();
    
    checksum += test_multiple_candidates(10, 5, 15);
    checksum += test_multiple_candidates(5, 10, 15);
    
    checksum += test_early_return_jump(25);
    checksum += test_early_return_jump(-5);
    
    checksum += test_loop_break_jump(10);
    checksum += test_loop_break_jump(50);
    
    checksum += test_switch_default_jump(1);
    checksum += test_switch_default_jump(2);
    checksum += test_switch_default_jump(5);  /* Goes to default */
#else
    /* Portable fallback tests */
    checksum += test_simple_jump_arithmetic_portable(10, 5);
    checksum += test_simple_jump_arithmetic_portable(5, 10);
    checksum += test_unconditional_jump_logical_portable(100, 200);
    
    /* Still run some tests that don't rely on delay slots */
    checksum += test_global_based_jump();
    checksum += test_loop_break_jump(10);
    checksum += test_switch_default_jump(3);
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0) ? 0 : 1;
}
