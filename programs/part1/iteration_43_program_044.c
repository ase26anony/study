/* test_delay_slots.c - Test program for GCC delay slot filling optimization */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
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
    result = b * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = a + 1;
    result = temp;
    
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int x, int y) {
    int val = x;
    
    /* Create a conditional jump to label */
    if (x > y) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    val = y - x;
    return val;
    
target2:
    /* Candidate: bitwise operation, no memory access */
    val = val ^ 0xFF;
    val = val + 1;
    
    return val;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int a, int b, int c) {
    int res = a;
    
    /* Force jump with register pressure */
    register int r1 asm("$8") = a;
    register int r2 asm("$9") = b;
    
    if (r1 > r2) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    res = c;
    return res;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    /* Use registers not used in jump condition */
    int t1 = r1 + r2;      /* Simple arithmetic */
    int t2 = t1 & 0x0F;    /* Bitwise operation */
    res = t2 * 2;
    
    return res;
}

/* Test 4: Nested jumps to create multiple opportunities */
int test_nested_jumps(int a, int b) {
    int result = 0;
    
    if (a == 0) {
        COMPILER_BARRIER();
        goto inner_label;
    }
    
    if (b > 0) {
        COMPILER_BARRIER();
        goto outer_label;
    }
    
    result = a + b;
    return result;
    
inner_label:
    /* First candidate location */
    result = a * 2;
    goto outer_label;
    
outer_label:
    /* Second candidate location */
    result = result | 0x01;
    
    return result;
}

/* Test 5: Function with return jump simulation */
int test_return_like_jump(int* ptr, int val) {
    int local = val;
    
    /* Create a jump that looks like a return */
    if (ptr != NULL && *ptr > 0) {
        COMPILER_BARRIER();
        goto return_label;
    }
    
    local = val * 3;
    return local;
    
return_label:
    /* Safe instruction: modify local variable only */
    local = local + *ptr;
    
    return local;
}

/* Test 6: Jump with safe memory operation at target */
int test_safe_memory_op(int* arr, int idx) {
    int sum = 0;
    
    if (idx >= 0 && idx < 10) {
        COMPILER_BARRIER();
        goto compute_label;
    }
    
    sum = -1;
    return sum;
    
compute_label:
    /* Safe memory access - array bounds already checked */
    int temp = arr[idx];
    sum = temp + idx;
    
    return sum;
}

/* Test 7: Complex jump pattern with register shuffling */
int test_register_shuffle(int a, int b, int c) {
    /* Use explicit register variables to control allocation */
    register int rA asm("$10") = a;
    register int rB asm("$11") = b;
    register int rC asm("$12") = c;
    register int rD asm("$13") = 0;
    
    if (rA > rB && rB > rC) {
        COMPILER_BARRIER();
        goto shuffle_label;
    }
    
    rD = rA + rB + rC;
    return rD;
    
shuffle_label:
    /* Instructions using different registers than condition */
    rD = rC + 1;        /* Use rC which isn't in the AND condition */
    rD = rD << 2;       /* Shift operation */
    
    return rD;
}

/* Main driver that exercises all tests */
int main(void) {
    int checksum = 0;
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    printf("Testing delay slot filling patterns (HAS_DELAY_SLOTS = %d)\n", 
           HAS_DELAY_SLOTS);
    
    /* Run all tests with various inputs */
    checksum += test_unconditional_jump(5, 3);
    checksum += test_conditional_jump(10, 5);
    checksum += test_conditional_jump(3, 7);
    checksum += test_multiple_candidates(8, 4, 2);
    checksum += test_nested_jumps(0, 5);
    checksum += test_nested_jumps(1, -1);
    checksum += test_return_like_jump(&array[0], 20);
    checksum += test_safe_memory_op(array, 3);
    checksum += test_register_shuffle(15, 10, 5);
    checksum += test_register_shuffle(5, 10, 15);
    
    /* Architecture-specific tests */
#if HAS_DELAY_SLOTS
    /* Additional patterns that might trigger specific reorg logic */
    {
        int a = 100, b = 50;
        
        /* Pattern with explicit asm to guide instruction selection */
        __asm__ volatile (
            "move %0, %1\n\t"
            "bne %1, $0, 1f\n\t"
            "nop\n\t"  /* Traditional delay slot */
            "j 2f\n\t"
            "nop\n"
            "1:\n\t"
            "addu %0, %0, 1\n\t"  /* Candidate for delay slot filling */
            "2:\n"
            : "=r"(a) : "r"(b) : "memory"
        );
        
        checksum += a;
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify expected results */
    int expected_min = 100;  /* Minimum reasonable checksum */
    if (checksum < expected_min) {
        printf("Warning: Checksum unexpectedly low\n");
        return 1;
    }
    
    return 0;
}
