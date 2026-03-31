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
#if HAS_DELAY_SLOTS
static int test_unconditional_jump(int a, int b) {
    int result = a;
    
    /* Create a simple jump pattern */
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
    int temp = result + 1;  /* Should be safe to move */
    result = temp;
    
    /* Additional code to prevent tail optimization */
    COMPILER_BARRIER();
    return result + 1000;
}
#else
static int test_unconditional_jump(int a, int b) {
    /* Portable version */
    return (a != 0) ? (a + 1 + 1000) : (b * 2);
}
#endif

/* Test 2: Conditional jump based on comparison */
#if HAS_DELAY_SLOTS
static int test_conditional_jump(int x, int y) {
    int val = x;
    
    /* Create conditional jump */
    if (x > y) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    val = y - x;
    return val;
    
target2:
    /* Candidate: bitwise operation on local variable */
    /* Should not conflict with jump resources */
    int local = val;
    local = local & 0xFF;  /* Simple bitmask */
    val = local;
    
    COMPILER_BARRIER();
    return val + 2000;
}
#else
static int test_conditional_jump(int x, int y) {
    return (x > y) ? ((x & 0xFF) + 2000) : (y - x);
}
#endif

/* Test 3: Jump with multiple candidate instructions at target */
#if HAS_DELAY_SLOTS
static int test_multi_candidate(int a, int b, int c) {
    int res = a;
    
    /* Jump based on multiple conditions */
    if ((a & 1) && (b > c)) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    res = c * 3;
    return res;
    
target3:
    /* Multiple simple instructions - one might be eligible */
    int t1 = res + b;
    int t2 = t1 ^ 0x1234;  /* XOR operation */
    int t3 = t2 << 2;      /* Shift operation */
    res = t3;
    
    COMPILER_BARRIER();
    return res + 3000;
}
#else
static int test_multi_candidate(int a, int b, int c) {
    return ((a & 1) && (b > c)) ? ((((a + b) ^ 0x1234) << 2) + 3000) : (c * 3);
}
#endif

/* Test 4: Nested control flow with jump to label */
#if HAS_DELAY_SLOTS
static int test_nested_jump(int x) {
    int value = x;
    
    /* More complex condition to create jump */
    switch (x % 4) {
        case 0:
        case 1:
            /* Fall through to jump */
            COMPILER_BARRIER();
            goto target4;
        default:
            value = x * x;
            return value;
    }
    
target4:
    /* Simple arithmetic that should be safe */
    value = value | 0x100;  /* OR operation */
    
    COMPILER_BARRIER();
    return value + 4000;
}
#else
static int test_nested_jump(int x) {
    return ((x % 4) == 0 || (x % 4) == 1) ? ((x | 0x100) + 4000) : (x * x);
}
#endif

/* Test 5: Function with return jump pattern */
#if HAS_DELAY_SLOTS
static int test_return_jump(int a, int flag) {
    int result = a;
    
    if (flag) {
        COMPILER_BARRIER();
        goto early_return;
    }
    
    /* Normal processing */
    result = a * a + 1;
    return result;
    
early_return:
    /* Instruction at jump target - simple increment */
    result = result + 5;
    
    COMPILER_BARRIER();
    return result + 5000;
}
#else
static int test_return_jump(int a, int flag) {
    return flag ? (a + 5 + 5000) : (a * a + 1);
}
#endif

/* Test 6: Avoid using special registers (like $ra on MIPS) */
#if HAS_DELAY_SLOTS
static int test_safe_registers(int x, int y) {
    /* Use local variables that don't conflict with return address */
    int local1 = x;
    int local2 = y;
    
    if (local1 < local2) {
        COMPILER_BARRIER();
        goto safe_target;
    }
    
    return local1 - local2;
    
safe_target:
    /* Use only local variables for the candidate instruction */
    int temp = local1 + local2;
    temp = temp * 2;
    
    COMPILER_BARRIER();
    return temp + 6000;
}
#else
static int test_safe_registers(int x, int y) {
    return (x < y) ? (((x + y) * 2) + 6000) : (x - y);
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    int i;
    
    /* Seed for reproducible tests */
    srand(42);
    
    /* Run multiple test cases with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        checksum += test_unconditional_jump(a, b);
        checksum += test_conditional_jump(a, b);
        checksum += test_multi_candidate(a, b, c);
        checksum += test_nested_jump(a);
        checksum += test_return_jump(a, i % 2);
        checksum += test_safe_registers(a, b);
    }
    
    printf("Delay slot test checksum: %d\n", checksum);
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO (using portable fallbacks)");
    
    return 0;
}
