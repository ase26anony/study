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
    
    /* Force a simple jump structure */
    if (x != 0) {
        /* This should become a simple unconditional jump */
        goto target1;
    }
    
    /* Dead code to make the jump necessary */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is available before the jump */
    
    /* Additional instructions to prevent tail merging */
    global_counter++;
    return result + global_counter;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Use different registers to avoid conflicts */
    if (a > b) {
        /* Force register usage before jump */
        int unused = temp1 * temp2;
        (void)unused;
        
        /* This should generate a conditional jump */
        goto target2;
    }
    
    /* Alternative path */
    temp1 = b - a;
    return temp1;
    
target2:
    /* Safe instruction: bitwise operation on local variable */
    temp1 = temp1 ^ 0x55;  /* Uses temp1 which is set before jump */
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile ("" : : : "memory");
    
    return temp1 + global_a;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    register int r1 asm("$8") = val;  /* Suggest register on MIPS */
    register int r2 asm("$9") = val * 2;
    
    if (r1 < 100) {
        /* Simple jump to label */
        goto target3;
    }
    
    r2 = r1 / 3;
    return r2;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    r1 = r1 + 5;      /* First candidate */
    r2 = r2 | 0xFF;   /* Second candidate */
    
    /* Use both results to prevent dead code elimination */
    return r1 + r2;
}

/* Test 4: Nested jumps to create different control flow */
int test_nested_control_flow(int x, int y, int z) {
    int result = x;
    
    if (x > y) {
        if (y > z) {
            /* Inner conditional jump */
            goto inner_target;
        }
        result = x - y;
    }
    
    /* Outer fallthrough */
    result = result * 2;
    return result;
    
inner_target:
    /* Safe instruction: logical operation */
    result = result & ~0x1;
    
    /* Jump again to prevent simple fallthrough */
    if (result != 0) {
        goto final_target;
    }
    
    return result;
    
final_target:
    /* Another safe instruction */
    result = result << 1;
    return result + global_b;
}

/* Test 5: Function with switch that generates jumps */
int test_switch_jump(int code) {
    int output = 0;
    
    switch (code & 0x3) {
        case 0:
            output = 1;
            break;
        case 1:
            /* This case should generate a jump to the target */
            goto switch_target;
        case 2:
            output = 3;
            break;
        default:
            output = 4;
            break;
    }
    
    return output;
    
switch_target:
    /* Instruction safe for delay slot */
    output = (code * 2) + 1;
    
    /* Prevent tail call optimization */
    global_counter += output;
    return output;
}

/* Test 6: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        if (i == limit - 1) {
            /* Early exit jump */
            goto loop_exit;
        }
        sum += i;
    }
    
    return sum;
    
loop_exit:
    /* Safe arithmetic at exit point */
    sum = sum * 2;
    
    /* Memory operation to create different pattern */
    global_a = sum;
    return sum + 1;
}

/* Test 7: Jump to label with only safe instructions */
int test_safe_sequence(int a, int b) {
    int local1 = a;
    int local2 = b;
    int local3 = a + b;  /* Temporary not used before jump */
    
    /* Force spill/reload pattern */
    if (local1 > 0 && local2 > 0) {
        /* Complex condition to prevent if-conversion */
        if ((local1 * local2) > 100) {
            goto safe_target;
        }
    }
    
    local3 = local1 - local2;
    return local3;
    
safe_target:
    /* Multiple independent instructions */
    local1 = local1 + 1;    /* Uses variable set before jump */
    local2 = local2 - 1;    /* Uses different variable */
    
    /* Use both to prevent optimization */
    return local1 * local2 + local3;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        global_a = i;
        global_b = i * 2;
        
        checksum += test_unconditional_jump(i, i + 1);
        checksum += test_conditional_jump(i, i + 2);
        checksum += test_multiple_candidates(i * 3);
        checksum += test_nested_control_flow(i, i + 1, i + 2);
        checksum += test_switch_jump(i);
        checksum += test_loop_exit(i + 3);
        checksum += test_safe_sequence(i, i + 4);
        
        /* Modify globals to affect control flow */
        global_counter++;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are non-deterministic to ensure execution */
    if (checksum == 0) {
        printf("Warning: All tests returned zero\n");
    }
    
    return 0;
}
