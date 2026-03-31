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
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump to a label */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Dead code to make the jump necessary */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is not live across the jump */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Use different registers for jump condition and target instruction */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Safe instruction: bitwise operation on local temporaries */
    result = temp1 ^ 0xFF;  /* Doesn't use temp2 which might be in needed set */
    global_counter += 2;
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force a jump */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local3 = local1 + local2;
    return local3;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    local1 = local1 + 5;      /* First candidate */
    local2 = local2 | 0x01;   /* Second candidate */
    local3 = local1 + local2;
    global_counter += 3;
    return local3;
}

/* Test 4: Nested jumps to create multiple opportunities */
int test_nested_jumps(int x, int y, int z) {
    int tmp = x;
    
    if (tmp > y) {
        COMPILER_BARRIER();
        goto outer_target;
    }
    
    tmp = y - z;
    return tmp;
    
outer_target:
    /* First instruction at outer target */
    tmp = tmp * 2;
    
    /* Another conditional jump inside the target block */
    if (tmp < 100) {
        COMPILER_BARRIER();
        goto inner_target;
    }
    
    tmp = tmp / 2;
    return tmp;
    
inner_target:
    /* Instruction at inner target - potential delay slot filler */
    tmp = tmp + 7;
    global_counter += 4;
    return tmp;
}

/* Test 5: Jump with safe memory operation at target */
int test_memory_operation(int *ptr, int idx) {
    int value = idx;
    
    if (ptr != NULL) {
        COMPILER_BARRIER();
        goto mem_target;
    }
    
    value = -1;
    return value;
    
mem_target:
    /* Safe memory operation: load from known-safe address */
    int safe_load = global_a;  /* Global is always safe */
    value = value + safe_load;
    global_counter += 5;
    return value;
}

/* Test 6: Function with switch that creates jumps to different labels */
int test_switch_jumps(int code) {
    int result = 0;
    
    switch (code & 0x3) {
        case 0:
            COMPILER_BARRIER();
            goto case0;
        case 1:
            result = 1;
            break;
        case 2:
            COMPILER_BARRIER();
            goto case2;
        default:
            result = -1;
            break;
    }
    
    return result;
    
case0:
    result = code << 2;  /* Shift operation - simple and safe */
    global_counter += 6;
    return result;
    
case2:
    result = code >> 1;  /* Another simple operation */
    global_counter += 7;
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit with jump */
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto early_exit;
        }
    }
    
    return sum;
    
early_exit:
    /* Simple arithmetic at exit target */
    sum = sum & 0x3FF;  /* Mask operation */
    global_counter += 8;
    return sum;
}

/* Test 8: Jump to label with register-only operations */
int test_register_only(int a, int b, int c) {
    /* Use different register sets for condition and target */
    int r1 = a;
    int r2 = b;
    int r3 = c;
    
    if (r1 > r2 && r2 > r3) {
        COMPILER_BARRIER();
        goto reg_target;
    }
    
    return r1 + r2 + r3;
    
reg_target:
    /* Only use r3 which wasn't in the condition */
    r3 = r3 * 3;
    global_counter += 9;
    return r3;
}

/* Main driver */
int main(void) {
    int checksum = 0;
    int array[4] = {10, 20, 30, 40};
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all tests */
    checksum += test_unconditional_jump(5, 3);
    checksum += test_conditional_jump(50, 25);
    checksum += test_multiple_candidates(42);
    checksum += test_nested_jumps(10, 5, 2);
    checksum += test_memory_operation(array, 2);
    checksum += test_switch_jumps(5);
    checksum += test_loop_exit(50);
    checksum += test_register_only(100, 50, 25);
    
    /* Additional architecture-specific tests */
#if HAS_DELAY_SLOTS
    /* More aggressive tests for delay slot architectures */
    printf("Running architecture-specific delay slot tests...\n");
    
    /* Force more jump patterns */
    for (int i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i*2);
        checksum += test_conditional_jump(i*10, i*5);
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0 && global_counter != 0) ? 0 : 1;
}
