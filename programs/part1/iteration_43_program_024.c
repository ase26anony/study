/* test_delay_slots.c - Test program for GCC delay slot filling optimization */

#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump to label */
    if (x != 0) {
        goto target1;
    }
    
    /* Some code to avoid fall-through optimization */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Should use register not live across jump */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Create conditional jump */
    if (a > b) {
        /* Use volatile asm to prevent optimization */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    temp1 = b - a;
    return temp1;
    
target2:
    /* Safe instruction: bitwise operation on local temps */
    temp2 = temp1 & 0xFF;  /* Shouldn't conflict with jump resources */
    return temp2;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force jump with simple condition */
    if (local1 < 100) {
        goto target3;
    }
    
    local2 = local1 / 2;
    return local2;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    local1 = local1 + 5;      /* First candidate */
    local2 = local1 | 0x01;   /* Second candidate */
    return local1 + local2;
}

/* Test 4: Nested jumps to create complex control flow */
int test_nested_jumps(int x, int y, int z) {
    int a = x, b = y, c = z;
    
    if (a > b) {
        if (b > c) {
            /* Simple jump to outer label */
            goto target4;
        }
        c = a + b;
    }
    
    b = c * 2;
    return b;
    
target4:
    /* Safe arithmetic with temporaries */
    a = b + c;  /* Uses registers that shouldn't be in jump's resource set */
    return a;
}

/* Test 5: Jump with memory operation (if architecture allows safe moves) */
int test_with_memory_op(int *ptr, int idx) {
    int value = idx;
    
    if (ptr != NULL && idx > 0) {
        goto target5;
    }
    
    value = -1;
    return value;
    
target5:
    /* Simple load from known-safe address */
    int temp = ptr[0];  /* Might be moved into delay slot if safe */
    value = temp + idx;
    return value;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int x) {
    int result = x;
    
    if (x == 0) {
        /* This creates a jump to return */
        goto early_return;
    }
    
    result = x * x;
    return result;
    
early_return:
    /* Instruction before return that could fill delay slot */
    result = 1;  /* Simple assignment */
    return result;
}

/* Test 7: Complex condition with simple target instruction */
int test_complex_condition(int a, int b, int c) {
    int t1 = a, t2 = b, t3 = c;
    
    /* Complex condition that simplifies to simple jump */
    if ((a > b) && (b < c) && (a != 0)) {
        __asm__ volatile ("" : : : "memory");
        goto target7;
    }
    
    t3 = a + b + c;
    return t3;
    
target7:
    /* Very simple instruction - good delay slot candidate */
    t1 = t2;  /* Register move */
    return t1;
}

/* Portable fallback implementations */
#if !HAS_DELAY_SLOTS
/* For architectures without delay slots, ensure same behavior */
int test_unconditional_jump(int x, int y) {
    return (x != 0) ? (x + 1) : (y * 2);
}

int test_conditional_jump(int a, int b) {
    return (a > b) ? (a & 0xFF) : (b - a);
}

int test_multiple_candidates(int val) {
    if (val < 100) {
        int local1 = val + 5;
        int local2 = local1 | 0x01;
        return local1 + local2;
    }
    return val / 2;
}

int test_nested_jumps(int x, int y, int z) {
    if (x > y && y > z) {
        return y + z;
    }
    return (x > y) ? ((x + y) * 2) : (z * 2);
}

int test_with_memory_op(int *ptr, int idx) {
    if (ptr != NULL && idx > 0) {
        return ptr[0] + idx;
    }
    return -1;
}

int test_return_jump(int x) {
    return (x == 0) ? 1 : (x * x);
}

int test_complex_condition(int a, int b, int c) {
    if ((a > b) && (b < c) && (a != 0)) {
        return b;
    }
    return a + b + c;
}
#endif

/* Main driver that exercises all test patterns */
int main(void) {
    int checksum = 0;
    int test_data[7][3] = {
        {5, 10, 0},      /* test_unconditional_jump */
        {15, 10, 0},     /* test_conditional_jump */
        {50, 0, 0},      /* test_multiple_candidates */
        {10, 5, 1},      /* test_nested_jumps */
        {0, 0, 0},       /* test_with_memory_op (ptr will be NULL) */
        {0, 0, 0},       /* test_return_jump */
        {10, 5, 8}       /* test_complex_condition */
    };
    
    /* Static array for memory test */
    static int memory_array[4] = {100, 200, 300, 400};
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run test 1 */
    checksum += test_unconditional_jump(test_data[0][0], test_data[0][1]);
    printf("Test 1 complete\n");
    
    /* Run test 2 */
    checksum += test_conditional_jump(test_data[1][0], test_data[1][1]);
    printf("Test 2 complete\n");
    
    /* Run test 3 */
    checksum += test_multiple_candidates(test_data[2][0]);
    printf("Test 3 complete\n");
    
    /* Run test 4 */
    checksum += test_nested_jumps(test_data[3][0], test_data[3][1], test_data[3][2]);
    printf("Test 4 complete\n");
    
    /* Run test 5 - with valid pointer */
    checksum += test_with_memory_op(memory_array, 1);
    printf("Test 5 complete\n");
    
    /* Run test 6 */
    checksum += test_return_jump(test_data[5][0]);
    printf("Test 6 complete\n");
    
    /* Run test 7 */
    checksum += test_complex_condition(test_data[6][0], test_data[6][1], test_data[6][2]);
    printf("Test 7 complete\n");
    
    printf("Final checksum: %d\n", checksum);
    
#if HAS_DELAY_SLOTS
    printf("Compiled for architecture with delay slots\n");
#else
    printf("Compiled for architecture without delay slots (using fallbacks)\n");
#endif
    
    return 0;
}
