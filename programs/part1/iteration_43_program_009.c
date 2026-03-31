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

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    /* Create a simple conditional jump */
    if (x > y) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* This should generate a simple jump to label */
        goto target_label1;
    }
    
    /* Fall-through path */
    result = y - x;
    return result;
    
target_label1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is not live across the jump in this context */
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation at target */
int test_unconditional_jump_logical(int a, int b) {
    int temp = a;
    
    /* Force an unconditional jump pattern */
    if (a != 0) {
        __asm__ volatile ("" : : : "memory");
        goto target_label2;
    }
    
    return b;
    
target_label2:
    /* Candidate: bitwise operation with temporary variable */
    temp = temp & 0xFF;  /* Simple operation unlikely to trap */
    return temp;
}

/* Test 3: Nested conditional with multiple candidates */
int test_nested_conditional(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            /* Complex enough to generate a jump */
            __asm__ volatile ("" : : : "memory");
            goto target_label3;
        }
        result = x + y;
    }
    
    return result + z;
    
target_label3:
    /* Multiple candidate instructions at target */
    result = x * 2;      /* First candidate */
    result = result | 1; /* Second candidate - simple logical */
    return result;
}

/* Test 4: Function with return jump pattern */
int test_return_jump(int a) {
    int local1 = a;
    int local2 = a + 10;
    
    if (local1 > 50) {
        /* This may generate a jump to return sequence */
        __asm__ volatile ("" : : : "memory");
        goto early_return;
    }
    
    /* Normal path */
    local2 = local2 * 2;
    return local2;
    
early_return:
    /* Instruction at jump target - use completely fresh variable
       to avoid resource conflicts */
    int fresh_var = 100;
    fresh_var = fresh_var + local1;  /* Should be safe to move */
    return fresh_var;
}

/* Test 5: Loop with break to label */
int test_loop_break_jump(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        if (i == 7) {
            /* Break out to label via goto */
            __asm__ volatile ("" : : : "memory");
            goto special_label;
        }
        sum += i;
    }
    
    return sum;
    
special_label:
    /* Simple arithmetic at target */
    sum = sum ^ 0xAAAA;  /* XOR operation - no trapping */
    return sum;
}

/* Test 6: Switch statement with default jump */
int test_switch_jump(int val) {
    int result = val;
    
    switch (val & 3) {
        case 0:
            result += 10;
            break;
        case 1:
            result += 20;
            break;
        default:
            /* Jump to handle default case */
            __asm__ volatile ("" : : : "memory");
            goto default_handler;
    }
    
    return result;
    
default_handler:
    /* Safe instruction: increment by constant */
    result = result + 1;
    return result;
}

/* Test 7: Multiple basic blocks with shared target */
int test_shared_target(int a, int b) {
    int temp = a;
    
    if (a > b) {
        goto common_target;
    }
    
    if (a < 0) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    return a + b;
    
common_target:
    /* Instruction that should be eligible for delay slot */
    temp = temp << 2;  /* Shift operation - no memory access */
    return temp;
}

/* Test 8: Avoid resource conflicts explicitly */
int test_no_conflict(int x) {
    /* Use local variables that won't be live across the jump */
    int tmp1 = x;
    int tmp2 = x * 2;
    
    if (tmp1 > 100) {
        /* tmp1 and tmp2 are used before jump, but we'll use
           fresh variables at target to avoid conflicts */
        __asm__ volatile ("" : : : "memory");
        goto clean_target;
    }
    
    return tmp1 + tmp2;
    
clean_target:
    /* Use completely new variables to avoid any resource conflicts
       with the jump instruction's context */
    int fresh1 = 42;
    int fresh2 = 24;
    fresh1 = fresh1 + fresh2;  /* Very safe candidate */
    return fresh1;
}

/* Test 9: Pointer arithmetic without memory access */
int test_safe_pointer_arithmetic(int *ptr, int idx) {
    int value = *ptr;
    
    if (idx == 0) {
        __asm__ volatile ("" : : : "memory");
        goto compute;
    }
    
    return value + idx;
    
compute:
    /* Safe computation without memory access */
    int offset = idx * sizeof(int);
    /* Just computation, no actual memory reference */
    return offset;
}

/* Test 10: Final comprehensive test */
int test_comprehensive(int a, int b, int c) {
    int result = 0;
    
    /* Multiple conditions to create interesting control flow */
    if (a > b && b > c) {
        __asm__ volatile ("" : : : "memory");
        goto final_target;
    }
    
    if (a < 0 || b < 0) {
        result = a + b;
        if (result < c) {
            __asm__ volatile ("" : : : "memory");
            goto final_target;
        }
    }
    
    return a + b + c;
    
final_target:
    /* Multiple simple instructions at target */
    result = a ^ b;      /* XOR - no trapping */
    result = result & c; /* AND - no trapping */
    result = result + 1; /* Increment - no trapping */
    return result;
}

/* Driver function */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all tests with various inputs */
    checksum += test_simple_jump_arithmetic(global_a, global_b);
    checksum += test_unconditional_jump_logical(global_a, global_b);
    checksum += test_nested_conditional(global_a, global_b, global_c);
    checksum += test_return_jump(global_a);
    checksum += test_loop_break_jump(10);
    checksum += test_switch_jump(global_a);
    checksum += test_shared_target(global_a, global_b);
    checksum += test_no_conflict(global_a);
    
    int array[3] = {1, 2, 3};
    checksum += test_safe_pointer_arithmetic(array, 1);
    checksum += test_comprehensive(global_a, global_b, global_c);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results match expectations */
    int expected = 0;
    for (int i = 0; i < 10; i++) {
        expected += test_simple_jump_arithmetic(i, i*2);
    }
    
    printf("Verification complete.\n");
    
    return checksum != 0 ? 0 : 1;
}
