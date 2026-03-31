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
volatile int global_a = 0, global_b = 0;

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        /* Force a jump to label */
        goto target1;
    }
    
    /* Some code that won't be executed in this path */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x, which is not live across the jump */
    
    /* Additional instructions to prevent tail optimization */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump with logical operation at target */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Create conditional jump */
    if (a > b) {
        /* Use inline asm as compiler barrier */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    temp1 = b - a;
    return temp1;
    
target2:
    /* Candidate: Logical operation with temporaries */
    temp1 = temp1 & 0xFF;  /* Simple operation, no memory access */
    temp2 = temp1 | 0x1;
    
    global_a = temp1;
    return temp2;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force jump based on external condition */
    if (global_counter % 2 == 0) {
        goto target3;
    }
    
    local1 = local1 / 2;
    return local1;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 + 5;      /* First candidate */
    local2 = local2 - 3;      /* Second candidate */
    
    /* Use both results to prevent dead code elimination */
    return local1 + local2;
}

/* Test 4: Nested jumps with safe target instructions */
int test_nested_jumps(int x, int y, int z) {
    int tmp = x;
    
    /* First level condition */
    if (x > 0) {
        /* Second level condition */
        if (y > 0) {
            /* This creates a simple jump to label */
            goto target4;
        }
        tmp = x + y;
    }
    
    tmp = tmp * z;
    return tmp;
    
target4:
    /* Very safe instruction: bit manipulation on local variable */
    tmp = tmp ^ 0xAAAA;  /* XOR with constant - no traps possible */
    
    /* Force register usage */
    __asm__ volatile ("" : "+r" (tmp));
    return tmp;
}

/* Test 5: Jump with register-only operations at target */
int test_register_only(int a, int b) {
    register int r1 asm("t0") = a;  /* Suggest temporary register */
    register int r2 asm("t1") = b;
    
    /* Create jump condition */
    if (r1 != r2) {
        __asm__ volatile ("" : : : "memory");
        goto target5;
    }
    
    r1 = r1 * r2;
    return r1;
    
target5:
    /* Register-only operations - no memory access */
    r1 = r1 << 2;    /* Shift left */
    r2 = r2 >> 1;    /* Shift right */
    
    /* Use results */
    return r1 + r2;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int x) {
    int result = x;
    
    /* Pattern that might create a simple jump for return */
    if (x > 100) {
        goto early_return;
    }
    
    result = x * 2;
    return result;
    
early_return:
    /* Safe instruction before return */
    result = result | 0x1;  /* Set LSB */
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit condition that creates a jump */
        if (sum > 1000) {
            goto exit_loop;
        }
    }
    
    return sum;
    
exit_loop:
    /* Instruction at jump target */
    sum = sum & 0x3FF;  /* Mask to 10 bits */
    return sum;
}

/* Test 8: Switch-like jump table pattern */
int test_switch_jump(int code) {
    int value = code;
    
    /* Simple switch-like pattern */
    switch (code & 0x3) {
        case 0:
            goto case0;
        case 1:
            value = value + 10;
            break;
        case 2:
            value = value * 2;
            break;
        case 3:
        default:
            goto case3;
    }
    
    return value;
    
case0:
    value = value ^ 0xFF;
    return value;
    
case3:
    value = value + 1;
    return value;
}

/* Main driver function */
int main() {
    int total = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        global_counter = i;
        
        total += test_unconditional_jump(i, i * 2);
        total += test_conditional_jump(i, i + 5);
        total += test_multiple_candidates(i * 3);
        total += test_nested_jumps(i, i - 1, i + 1);
        total += test_register_only(i, i * 7);
        total += test_return_jump(i * 10);
        total += test_loop_exit(i + 5);
        total += test_switch_jump(i);
        
        /* Update globals to affect control flow */
        global_a = total & 0xFF;
        global_b = (total >> 8) & 0xFF;
    }
    
    printf("Final checksum: %d\n", total);
    
    /* Verify results are non-deterministic to ensure all paths executed */
    if (total != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Warning: All tests returned zero.\n");
        return 1;
    }
}
