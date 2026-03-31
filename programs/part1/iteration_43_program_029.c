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
        /* Force compiler to generate a jump */
        goto target1;
    }
    
    /* Some code to avoid fall-through optimization */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump context */
    result = x + 1;  /* Should be safe to move into delay slot */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Use different registers to avoid conflicts */
    if (temp1 > temp2) {
        /* This should generate a conditional jump */
        goto target2;
    }
    
    temp1 = temp2 * 3;
    return temp1;
    
target2:
    /* Candidate: bitwise operation with local temps */
    temp1 = temp1 ^ 0xFF;  /* Safe operation */
    global_a = temp1;
    return temp1;
}

/* Test 3: Jump with multiple candidate instructions */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    if (local1 < 100) {
        goto target3;
    }
    
    local2 = local1 - 50;
    return local2;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 + 5;    /* First candidate */
    local2 = local2 | 0x01; /* Second candidate */
    global_b = local1;
    return local1 + local2;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int tmp = x;
    
    if (x > y) {
        if (y > z) {
            /* Inner conditional jump */
            goto inner_target;
        }
        tmp = z;
    }
    
    tmp = tmp * 2;
    return tmp;
    
inner_target:
    /* Safe arithmetic with fresh register usage */
    tmp = tmp + z;  /* Uses z which isn't live across the jump? */
    return tmp;
}

/* Test 5: Function with switch and goto to create jump table */
int test_switch_jump(int code) {
    int value = code * 2;
    
    switch (code & 0x3) {
        case 0:
            goto case0;
        case 1:
            value += 10;
            break;
        case 2:
            goto case2;
        default:
            value -= 5;
            break;
    }
    
    return value;
    
case0:
    value = value << 1;  /* Shift operation - good candidate */
    return value;
    
case2:
    value = value & ~0x1;  /* Bit clear operation */
    return value;
}

/* Test 6: Avoid resource conflicts by using fresh variables */
int test_fresh_variables(int base) {
    /* Create fresh variables right before jump target */
    int fresh1, fresh2;
    
    if (base % 2 == 0) {
        fresh1 = base;
        fresh2 = 42;
        goto fresh_target;
    }
    
    return base * 3;
    
fresh_target:
    /* These use variables defined right before the label,
       minimizing resource conflicts */
    fresh1 = fresh1 + fresh2;
    return fresh1;
}

/* Test 7: Complex expression that might split */
int test_complex_expression(int a, int b) {
    int result = a;
    
    if (a != b) {
        goto complex_target;
    }
    
    result = b * a;
    return result;
    
complex_target:
    /* Complex enough that try_split might be called,
       but simple enough to be eligible */
    result = (a + b) * 2 - 1;
    return result;
}

/* Test 8: Memory operation that should be safe */
int test_safe_memory(int *ptr) {
    int local = *ptr;
    
    if (local > 0) {
        goto memory_target;
    }
    
    return local;
    
memory_target:
    /* Memory store to global - should be safe if not trapping */
    global_counter = local;
    return local + 1;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int test_data[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all test functions */
    checksum += test_unconditional_jump(test_data[0], test_data[1]);
    checksum += test_conditional_jump(test_data[1], test_data[2]);
    checksum += test_multiple_candidates(test_data[2]);
    checksum += test_nested_jumps(test_data[3], test_data[4], test_data[5]);
    checksum += test_switch_jump(test_data[0]);
    checksum += test_fresh_variables(test_data[6]);
    checksum += test_complex_expression(test_data[5], test_data[7]);
    
    /* Test with memory operation */
    checksum += test_safe_memory(&test_data[0]);
    
    printf("Final checksum: %d\n", checksum);
    printf("Global state: counter=%d, a=%d, b=%d\n", 
           global_counter, global_a, global_b);
    
    /* Verify results are consistent */
    if (checksum != (test_unconditional_jump(test_data[0], test_data[1]) +
                     test_conditional_jump(test_data[1], test_data[2]) +
                     test_multiple_candidates(test_data[2]) +
                     test_nested_jumps(test_data[3], test_data[4], test_data[5]) +
                     test_switch_jump(test_data[0]) +
                     test_fresh_variables(test_data[6]) +
                     test_complex_expression(test_data[5], test_data[7]) +
                     test_safe_memory(&test_data[0]))) {
        printf("ERROR: Checksum verification failed!\n");
        return 1;
    }
    
    return 0;
}
