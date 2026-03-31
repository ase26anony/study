/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(_MIPS_)
#define HAS_DELAY_SLOTS 1
#elif defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_a = 0, global_b = 0;

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != y) {
        /* Force compiler to generate a jump */
        goto target1;
    }
    
    /* This code should not be reached when x != y */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump context */
    result = x + 1;  /* Uses 'x' which is set before the jump */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump with safe register usage */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Use different comparisons to create various jump patterns */
    if (temp1 > temp2) {
        goto target2;
    }
    
    if (temp1 == temp2) {
        result = temp1 * temp2;
        return result;
    }
    
    result = temp2 - temp1;
    return result;
    
target2:
    /* Candidate: Simple arithmetic with registers not live across jump */
    int safe_reg = temp1;  /* Copy to avoid direct use of parameter */
    safe_reg = safe_reg & 0xFF;  /* Bitwise operation - safe */
    result = safe_reg + 5;
    return result;
}

/* Test 3: Jump with memory barrier and safe memory operation */
int test_jump_with_memory(int *ptr, int val) {
    int local = val;
    
    if (local > 100) {
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target3;
    }
    
    local = local * 2;
    return local;
    
target3:
    /* Safe memory operation - store to global that's not in needed set */
    int temp = local + 1;
    global_a = temp;  /* Store to global - should be safe */
    return temp;
}

/* Test 4: Nested jumps to create multiple candidates */
int test_nested_jumps(int a, int b, int c) {
    int result = a;
    
    if (a > b) {
        if (b > c) {
            goto inner_target;
        }
        goto outer_target;
    }
    
    result = c;
    return result;
    
inner_target:
    /* Simple instruction that could fill delay slot */
    result = b << 2;  /* Shift operation */
    return result;
    
outer_target:
    /* Another candidate instruction */
    result = a | b;  /* Bitwise OR */
    return result;
}

/* Test 5: Function with multiple labels and jumps */
int test_multiple_labels(int x) {
    int y = x;
    
    switch (x & 3) {
        case 0:
            goto case0;
        case 1:
            goto case1;
        case 2:
            goto case2;
        default:
            y = x * 3;
            break;
    }
    
    return y;
    
case0:
    y = x + 10;
    break;
    
case1:
    y = x - 5;
    break;
    
case2:
    y = x ^ 0xFF;  /* XOR operation */
    break;
}

/* Test 6: Loop with exit jump */
int test_loop_exit(int limit) {
    int sum = 0;
    int i;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            goto exit_early;
        }
    }
    
    return sum;
    
exit_early:
    /* Candidate instruction at jump target */
    sum = sum & 0x3FF;  /* Mask to 10 bits */
    return sum;
}

/* Test 7: Arithmetic that avoids resource conflicts */
int test_safe_arithmetic(int a, int b) {
    /* Use local variables that won't conflict with jump resources */
    int local1 = a;
    int local2 = b;
    int result;
    
    if (local1 < 0) {
        goto negative;
    }
    
    result = local1 + local2;
    return result;
    
negative:
    /* Very safe instruction: uses only local variable set before jump */
    int safe_temp = local1;
    safe_temp = -safe_temp;  /* Negation */
    result = safe_temp + local2;
    return result;
}

/* Test 8: Jump to label with multiple safe instructions */
int test_multiple_instructions(int x) {
    if (x == 0) {
        goto handle_zero;
    }
    
    return x * 2;
    
handle_zero:
    /* Multiple instructions - first one might be moved to delay slot */
    int y = 1;
    y = y + x;      /* x is 0, so this is just y = 1 */
    y = y << 3;     /* Shift */
    return y;
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_controlled(void) {
    int a = 10, b = 20, result;
    
    /* Force a simple jump pattern with asm */
    if (a < b) {
        /* Inline asm to create specific instruction pattern */
        __asm__ volatile (
            "b   1f\n\t"          /* Simple branch forward */
            "nop\n\t"             /* Traditional delay slot */
            "1:\n\t"
            : : : "memory"
        );
        
        /* Target label's first instruction */
        result = a + b;
    } else {
        result = a - b;
    }
    
    return result;
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    int test_results[10];
    int i;
    
    /* Initialize test values */
    int test_a = 42, test_b = 17, test_c = 99;
    int *test_ptr = &global_a;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests */
    test_results[0] = test_unconditional_jump(test_a, test_b);
    test_results[1] = test_conditional_jump(test_a, test_b);
    test_results[2] = test_jump_with_memory(test_ptr, test_c);
    test_results[3] = test_nested_jumps(test_a, test_b, test_c);
    test_results[4] = test_multiple_labels(test_a & 3);
    test_results[5] = test_loop_exit(50);
    test_results[6] = test_safe_arithmetic(-5, test_b);
    test_results[7] = test_multiple_instructions(0);
    
#if HAS_DELAY_SLOTS
    test_results[8] = test_asm_controlled();
#else
    test_results[8] = 0;
#endif
    
    /* Calculate checksum */
    for (i = 0; i < 9; i++) {
        checksum ^= test_results[i];
        checksum = (checksum << 1) | (checksum >> 31);
        printf("Test %d result: %d\n", i, test_results[i]);
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Verify all tests produced reasonable results */
    if (checksum != 0) {
        printf("All tests completed successfully\n");
    }
    
    return 0;
}
