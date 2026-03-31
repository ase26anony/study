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

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        /* This should become a simplejump_p */
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
    
    /* Create a conditional jump that should be simplejump_p */
    if (temp1 > temp2) {
        /* Force compiler to generate a jump to label */
        goto target2;
    }
    
    /* Alternative path */
    temp1 = temp2 - 5;
    return temp1;
    
target2:
    /* Candidate instruction: logical operation with temporaries */
    temp1 = temp1 & 0xFF;  /* Safe operation on local variable */
    temp2 = temp2 | 0x1;
    return temp1 + temp2;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Use volatile to prevent reordering */
    if (global_a > global_b) {
        __asm__ volatile ("" : : : "memory");
        goto target3;
    }
    
    local1 = local2 / 3;
    return local1;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 + 1;      /* First candidate */
    local2 = local2 - 1;      /* Second candidate */
    __asm__ volatile ("" : : : "memory");  /* Barrier */
    return local1 * local2;
}

/* Test 4: Nested jumps to create different flow patterns */
int test_nested_jumps(int x, int y, int z) {
    int tmp = x;
    
    if (x > y) {
        if (y > z) {
            /* Inner conditional jump */
            goto inner_target;
        }
        tmp = z;
    }
    
    /* Outer label */
    goto outer_end;
    
inner_target:
    /* Instruction that should be safe for delay slot */
    tmp = tmp ^ 0x55;  /* Bitwise operation on local */
    /* Another jump to create more opportunities */
    if (tmp > 0) {
        goto outer_end;
    }
    tmp = -tmp;
    
outer_end:
    return tmp + global_counter;
}

/* Test 5: Function with switch that generates jumps */
int test_switch_jumps(int code) {
    int result = 0;
    
    switch (code & 0x3) {
        case 0:
            goto case0;
        case 1:
            result = 1;
            break;
        case 2:
            goto case2;
        default:
            result = -1;
            break;
    }
    
    return result;
    
case0:
    /* Safe arithmetic at jump target */
    result = code + 100;
    /* Jump back to common code */
    goto switch_end;
    
case2:
    /* Another safe instruction */
    result = code * 2;
    goto switch_end;
    
switch_end:
    return result;
}

/* Test 6: Avoid using return address register (important for MIPS $ra) */
int test_no_ra_conflict(int a, int b) {
    /* Use local registers that aren't special */
    register int r1 asm("t0") = a;  /* Suggest temporary reg on MIPS */
    register int r2 asm("t1") = b;
    
    if (r1 != r2) {
        /* Compiler barrier to prevent optimization */
        __asm__ volatile ("" : : : "memory");
        goto no_ra_target;
    }
    
    return r1 - r2;
    
no_ra_target:
    /* Operation on temporary registers only */
    r1 = r1 + r2;
    r2 = r2 ^ r1;
    return r1 | r2;
}

/* Test 7: Memory operation that should be safe (non-trapping) */
int test_safe_memory(int *ptr, int idx) {
    int local_array[4] = {1, 2, 3, 4};
    int result = 0;
    
    /* Ensure idx is in bounds to avoid trapping */
    idx = idx & 0x3;
    
    if (ptr != NULL) {
        goto mem_target;
    }
    
    result = local_array[idx];
    return result;
    
mem_target:
    /* Safe memory access to local array */
    result = local_array[idx] + 1;
    /* Also do a safe store */
    local_array[idx] = result;
    return result;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int test_results[8];
    int *dummy_ptr = &global_a;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run all test functions */
    test_results[0] = test_unconditional_jump(10, 20);
    test_results[1] = test_conditional_jump(50, 30);
    test_results[2] = test_multiple_candidates(7);
    test_results[3] = test_nested_jumps(5, 3, 8);
    test_results[4] = test_switch_jumps(0);
    test_results[5] = test_switch_jumps(2);
    test_results[6] = test_no_ra_conflict(12, 34);
    test_results[7] = test_safe_memory(dummy_ptr, 1);
    
    /* Calculate checksum to ensure all code executed */
    for (int i = 0; i < 8; i++) {
        checksum += test_results[i];
        printf("Test %d result: %d\n", i, test_results[i]);
    }
    
    printf("Total checksum: %d\n", checksum);
    
    /* Architecture-specific verification */
#if HAS_DELAY_SLOTS
    printf("Compiled for architecture with delay slots\n");
    printf("Check RTL dump with: -fdump-rtl-reorg to see delay slot filling\n");
#endif
    
    return (checksum != 0) ? 0 : 1;
}

/* Additional function to create more jump opportunities */
void extra_jump_patterns(void) {
    static int counter = 0;
    
    /* Pattern 1: Loop with conditional exit */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            goto loop_label;
        }
        counter++;
    }
    return;
    
loop_label:
    counter += 100;
    
    /* Pattern 2: Tail recursion simulation */
    int x = 0;
    if (counter > 50) {
        x = 1;
        goto tail_call;
    }
    
tail_call:
    x = x * 2;
    (void)x;
}
