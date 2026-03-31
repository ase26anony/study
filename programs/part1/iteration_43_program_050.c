/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__MIPS__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_a = 0, global_b = 0;

/* Barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple unconditional jump pattern */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Should use register not live across jump */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Use different registers to avoid conflicts */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Candidate: Bitwise operation with temporaries */
    result = temp1 ^ temp2;  /* XOR uses different resources than comparison */
    result = result & 0xFF;  /* Additional simple operation */
    global_a = temp1;
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
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 + 5;      /* First candidate */
    local2 = local2 | 0x01;   /* Second candidate - logical OR */
    local3 = local1 + local2; /* Third candidate - addition */
    global_b = local3;
    return local3;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int tmp = x;
    
    if (tmp > y) {
        if (tmp > z) {
            COMPILER_BARRIER();
            goto target4a;
        } else {
            COMPILER_BARRIER();
            goto target4b;
        }
    }
    
    tmp = y + z;
    return tmp;
    
target4a:
    /* Simple increment - good delay slot candidate */
    tmp = tmp + 1;
    return tmp;
    
target4b:
    /* Shift operation - another good candidate */
    tmp = tmp << 2;
    return tmp;
}

/* Test 5: Jump with memory operation at target (safe load) */
int test_memory_op(int *ptr, int idx) {
    int value = 0;
    
    if (ptr != NULL) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    value = idx * 10;
    return value;
    
target5:
    /* Load from known-safe memory location */
    value = ptr[idx & 0x3];  /* Mask to ensure safe access */
    /* Follow with arithmetic to increase candidate value */
    value = value + global_counter;
    return value;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int a, int b) {
    int sum = a + b;
    
    if (sum > 100) {
        COMPILER_BARRIER();
        goto early_return;
    }
    
    /* Normal processing */
    sum = sum * 2;
    return sum;
    
early_return:
    /* Simple instruction before return */
    sum = sum | 0x80000000;  /* Set high bit */
    return sum;
}

/* Architecture-specific targeting */
#if HAS_DELAY_SLOTS

/* Test 7: Explicit assembly to guide instruction selection on MIPS/SPARC */
int test_assembly_hint(int x) {
    int result = x;
    
    /* Force a simple jump pattern */
    if (result != 0) {
        /* Inline asm to prevent optimization of jump pattern */
        __asm__ volatile(
            ".set noreorder\n\t"
            ".set nomacro\n\t"
            "bne $0, %0, 1f\n\t"
            "nop\n\t"  /* Traditional delay slot nop */
            ".set reorder\n\t"
            ".set macro\n\t"
            : : "r"(result) : "memory"
        );
        
        goto asm_target;
    }
    
    result = x * 3;
    return result;
    
asm_target:
    /* Very simple instruction that should be delay-slot eligible */
    result = result & ~1;  /* Clear LSB */
    return result;
}

/* Test 8: Multiple basic blocks to create jump opportunities */
int test_complex_flow(int a, int b, int c) {
    int r1 = a, r2 = b, r3 = c;
    
    /* Create multiple jump possibilities */
    if (r1 > r2) {
        if (r2 > r3) {
            COMPILER_BARRIER();
            goto label8a;
        } else {
            COMPILER_BARRIER();
            goto label8b;
        }
    } else if (r1 < r3) {
        COMPILER_BARRIER();
        goto label8c;
    }
    
    r1 = r2 + r3;
    return r1;
    
label8a:
    r1 = r1 * r1;
    return r1;
    
label8b:
    r2 = r2 - r3;
    return r2;
    
label8c:
    r3 = r3 ^ 0x5555;
    return r3;
}

#endif /* HAS_DELAY_SLOTS */

/* Main driver function */
int main() {
    int checksum = 0;
    int array[4] = {10, 20, 30, 40};
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        checksum ^= test_unconditional_jump(i, i*2);
        checksum ^= test_conditional_jump(i, i+5);
        checksum ^= test_multiple_candidates(i*3);
        checksum ^= test_nested_jumps(i, i+1, i+2);
        checksum ^= test_memory_op(array, i & 3);
        checksum ^= test_return_jump(i, i*4);
        
        #if HAS_DELAY_SLOTS
        checksum ^= test_assembly_hint(i);
        checksum ^= test_complex_flow(i, i+3, i+6);
        #endif
        
        /* Modify globals to affect future iterations */
        global_counter++;
        global_a ^= i;
        global_b += i;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d, Global A: %d, Global B: %d\n", 
           global_counter, global_a, global_b);
    
    return checksum != 0 ? 0 : 1;
}
