/* test_delay_slots.c - Test program for GCC delay slot filling optimization */

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
    
    /* Force a simple jump structure */
    if (a != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    return b;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = a + 1;  /* Should use registers not live across jump */
    
    /* Additional code to prevent tail optimization */
    result = result * 2;
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
    /* Candidate: bitwise operation on safe temporary */
    temp = temp ^ 0xFF;  /* XOR with constant */
    
    /* More operations to make it non-trivial */
    temp = temp + (y << 2);
    return temp;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int val = a;
    
    /* Force jump with register pressure */
    if (c != 0) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    val = b;
    return val;
    
target3:
    /* Sequence of simple instructions, any of which could be candidate */
    val = val + b;      /* First candidate - addition */
    val = val & 0x0F;   /* Second candidate - bitwise AND */
    val = val | c;      /* Third candidate - bitwise OR */
    
    return val;
}

/* Test 4: Nested jumps to create complex flow */
int test_nested_jumps(int p, int q) {
    int res = p;
    
    if (p > 100) {
        COMPILER_BARRIER();
        goto outer_target;
    }
    
    return q;
    
outer_target:
    /* First instruction at outer target */
    res = res - q;
    
    /* Inner conditional */
    if (res < 0) {
        COMPILER_BARRIER();
        goto inner_target;
    }
    
    res = res + 50;
    return res;
    
inner_target:
    /* Candidate at inner target */
    res = res * 2;
    return res;
}

/* Test 5: Jump with memory operation (load) at target */
int test_with_load(int *ptr, int idx) {
    int value = idx;
    
    if (ptr != NULL) {
        COMPILER_BARRIER();
        goto load_target;
    }
    
    return -1;
    
load_target:
    /* Load from memory - might be eligible if no resource conflicts */
    value = ptr[idx];
    
    /* Simple arithmetic to ensure it's not just a load */
    value = value + idx;
    return value;
}

/* Test 6: Function with switch statement creating multiple jumps */
int test_switch_jumps(int code) {
    int result = 0;
    
    switch (code & 0x3) {
        case 0:
            COMPILER_BARRIER();
            goto case0_target;
        case 1:
            result = 1;
            break;
        case 2:
            result = 2;
            break;
        case 3:
            result = 3;
            break;
    }
    
    return result;
    
case0_target:
    /* Candidate instruction for delay slot */
    result = (code << 1) | 0x1;
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit condition creating a jump */
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto early_exit;
        }
    }
    
    return sum;
    
early_exit:
    /* Candidate at early exit target */
    sum = sum * 2;
    return sum;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all test functions with various inputs */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(50, 30);
    checksum += test_multiple_candidates(5, 7, 3);
    checksum += test_nested_jumps(150, 25);
    checksum += test_with_load(array, 5);
    checksum += test_switch_jumps(0);
    checksum += test_loop_exit(50);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are reasonable */
    if (checksum < 0) {
        printf("Error: Negative checksum\n");
        return 1;
    }
    
    return 0;
}
