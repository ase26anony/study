/* test_delay_slots.c - Trigger GCC's delay slot filling optimization */
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
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
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
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Use distinct temporary variables to avoid resource conflicts */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    temp1 = temp2 - 5;
    return temp1;
    
target2:
    /* Candidate: Bitwise operation with safe temporaries */
    temp1 = temp1 ^ 0xFF;  /* XOR with constant */
    return temp1;
}

/* Test 3: Jump with multiple safe instructions at target */
int test_multiple_instructions(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force a jump */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local1 = local2 + 10;
    return local1;
    
target3:
    /* Multiple simple instructions - one might be eligible */
    local1 = local1 + 2;    /* First candidate */
    local2 = local1 & 0x0F; /* Second candidate */
    return local2;
}

/* Test 4: Nested control flow with jumps */
int test_nested_jumps(int x, int y, int z) {
    int a = x, b = y, c = z;
    
    if (a > b) {
        if (b > c) {
            COMPILER_BARRIER();
            goto target4;
        }
        a = c;
    }
    
    return a + b;
    
target4:
    /* Safe arithmetic with fresh temporaries */
    int t1 = a + b;
    int t2 = t1 * 2;
    return t2;
}

/* Test 5: Jump with logical operations at target */
int test_logical_ops(int p, int q) {
    int r = p;
    int s = q;
    
    /* Create jump condition */
    if ((p & 1) == 0) {  /* Even number */
        COMPILER_BARRIER();
        goto target5;
    }
    
    r = s | 0x100;
    return r;
    
target5:
    /* Logical operations that are safe to move */
    r = r << 2;     /* Shift left */
    s = r | s;      /* OR operation */
    return s;
}

/* Test 6: Function with return jump pattern */
int test_return_jump(int base) {
    int accum = base;
    
    if (accum > 0) {
        COMPILER_BARRIER();
        goto compute;
    }
    
    return accum * (-1);
    
compute:
    /* Simple computation that doesn't use special registers */
    accum = accum + 10;
    accum = accum * 2;
    return accum;
}

/* Architecture-specific targeting */
#if HAS_DELAY_SLOTS
/* Test specifically crafted for delay slot architectures */
int test_delay_slot_target(int seed) {
    register int r1 asm("$t0") = seed;  /* Suggest temporary register */
    register int r2 asm("$t1") = seed + 1;
    
    /* Force a simple jump to label */
    if (r1 != 0) {
        /* Inline asm to ensure jump is generated */
        __asm__ volatile(
            "bne %0, $zero, 1f\n\t"
            "nop\n\t"
            "1:\n\t"
            : : "r"(r1) : "memory"
        );
        goto ds_target;
    }
    
    r2 = r1 * 3;
    return r2;
    
ds_target:
    /* Ideal candidate for delay slot filling:
       - Uses temporary registers
       - Simple arithmetic
       - No memory access
       - No function calls */
    r1 = r1 + r2;
    return r1;
}
#endif

/* Main driver */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns...\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests */
    checksum += test_unconditional_jump(10, 20);
    checksum += test_conditional_jump(50, 30);
    checksum += test_multiple_instructions(25);
    checksum += test_nested_jumps(100, 50, 25);
    checksum += test_logical_ops(64, 32);
    checksum += test_return_jump(5);
    
#if HAS_DELAY_SLOTS
    checksum += test_delay_slot_target(7);
    printf("Delay-slot-specific test included.\n");
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are non-zero */
    if (checksum == 0) {
        printf("ERROR: All tests returned zero!\n");
        return 1;
    }
    
    return 0;
}
