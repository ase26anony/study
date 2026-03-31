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
    
    /* Force a simple jump to label */
    if (x != 0) {
        goto target1;
    }
    
    /* This code should not be reached when x != 0 */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variables not live across the jump */
    int temp = x + 1;  /* Should not conflict with jump resources */
    result = temp;
    
    /* Additional instructions to prevent tail optimization */
    global_counter += result;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int local1 = a;
    int local2 = b;
    int result = 0;
    
    /* Create a conditional jump */
    if (local1 > local2) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    result = local2 - local1;
    global_a = result;
    return result;
    
target2:
    /* Candidate: bitwise operation on fresh temporaries */
    int temp1 = local1;
    int temp2 = local2;
    temp1 = temp1 ^ temp2;  /* XOR operation - non-trapping */
    result = temp1;
    
    global_b = result;
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int tmp = val;
    int result = 0;
    
    if (tmp % 2 == 0) {
        goto target3;
    }
    
    result = tmp * 3;
    return result;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    int t1 = tmp + 5;
    int t2 = t1 << 2;    /* Shift operation */
    int t3 = t2 & 0xFF;  /* Mask operation */
    result = t3;
    
    /* Use result to prevent dead code elimination */
    global_counter += result;
    return result;
}

/* Test 4: Nested jumps with safe target instruction */
int test_nested_control(int x, int y, int z) {
    int a = x, b = y, c = z;
    int result = 0;
    
    if (a > 0) {
        if (b > 0) {
            /* Simple jump to label */
            goto target4;
        }
        result = a + b;
        return result;
    }
    
    result = c;
    return result;
    
target4:
    /* Safe instruction: increment local variable */
    c = c + 1;  /* Uses variable not referenced before jump */
    result = a + b + c;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile ("" : : : "memory");
    return result;
}

/* Test 5: Function with return-like jump pattern */
int test_return_pattern(int base) {
    int value = base;
    
    if (value > 100) {
        goto early_exit;
    }
    
    /* Normal processing */
    value = value * 2 + 1;
    return value;
    
early_exit:
    /* Simple arithmetic that doesn't use special registers */
    int adjustment = 10;
    value = value + adjustment;
    
    return value;
}

/* Test 6: Jump to label with register-only operations */
int test_register_only(int p, int q) {
    register int r1 asm("t0") = p;  /* Suggest temporary register */
    register int r2 asm("t1") = q;
    int result;
    
    if (r1 != r2) {
        __asm__ volatile ("" : : : "memory");
        goto reg_target;
    }
    
    result = r1 * r2;
    return result;
    
reg_target:
    /* Register-to-register operation - ideal for delay slot */
    r1 = r1 + r2;
    result = r1;
    
    return result;
}

/* Test 7: Avoid memory references at target (no loads/stores) */
int test_no_memory_refs(int x) {
    int a = x;
    int b = x * 2;
    
    if (a < 0) {
        goto no_mem_target;
    }
    
    return b;
    
no_mem_target:
    /* Pure computation - no memory access */
    int c = a ^ b;      /* XOR */
    int d = c << 3;     /* Shift */
    int e = d | 0x1F;   /* OR with immediate */
    
    return e;
}

/* Architecture-specific test that uses inline asm for precise control */
#if HAS_DELAY_SLOTS
int test_asm_guided(void) {
    int x = 5, y = 3, result;
    
    /* Force a simple jump pattern using inline asm */
    __asm__ volatile (
        "move $t0, %1\n\t"
        "move $t1, %2\n\t"
        "bne $t0, $t1, 1f\n\t"
        "nop\n\t"
        "b 2f\n\t"
        "nop\n\t"
        "1:\n\t"
        "addu $t2, $t0, $t1\n\t"
        "move %0, $t2\n\t"
        "2:\n\t"
        : "=r" (result)
        : "r" (x), "r" (y)
        : "t0", "t1", "t2", "memory"
    );
    
    return result;
}
#endif

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run test multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i*2);
        checksum += test_conditional_jump(i, i+1);
        checksum += test_multiple_candidates(i);
        checksum += test_nested_control(i, i-1, i+1);
        checksum += test_return_pattern(i * 10);
        checksum += test_register_only(i, i+2);
        checksum += test_no_memory_refs(i);
        
        #if HAS_DELAY_SLOTS
        checksum += test_asm_guided();
        #endif
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global a: %d, Global b: %d\n", global_a, global_b);
    
    return checksum != 0 ? 0 : 1;
}
