/* delay_slot_test.c - Test program for GCC delay slot filling optimization */
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
    
    /* Some code that won't be reached if x != 0 */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Should use registers not live across the jump */
    
    /* Additional instructions to prevent tail optimization */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    
    /* Use different registers for jump condition and target instruction */
    if (temp1 > temp2) {
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    temp1 = temp1 - temp2;
    return temp1;
    
target2:
    /* Safe instruction: bitwise operation on local variable */
    temp1 = temp1 ^ 0xFF;  /* XOR with constant - safe */
    
    /* Use result to prevent dead code elimination */
    global_a = temp1;
    return temp1 + 1;
}

/* Test 3: Jump with multiple safe instructions at target */
int test_multiple_instructions(int val) {
    int local1 = val;
    int local2 = val * 2;
    
    /* Force jump generation */
    if (local1 < 100) {
        __asm__ volatile("" : : : "memory");
        goto target3;
    }
    
    local2 = local2 / 3;
    return local2;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local1 = local1 | 0x01;    /* Bitwise OR - no traps */
    local2 = local2 & 0xFE;    /* Bitwise AND - no traps */
    
    /* Ensure both results are used */
    return local1 + local2;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int result = x;
    
    if (x > y) {
        if (y > z) {
            /* Two-level jump structure */
            __asm__ volatile("" : : : "memory");
            goto inner_target;
        }
        result = x - y;
    }
    
    /* Outer fallthrough */
    result = result + z;
    return result;
    
inner_target:
    /* Very safe instruction: increment with constant */
    result = result + 1;
    
    /* Additional computation to prevent optimization */
    global_b = result;
    return result * 2;
}

/* Test 5: Function with switch-like jump table (simplified) */
int test_switch_jump(int code) {
    int output = 0;
    
    /* Simple switch that might generate jump instructions */
    switch (code & 0x3) {  /* Mask to limit cases */
        case 0:
            __asm__ volatile("" : : : "memory");
            goto case_target;
        case 1:
            output = 1;
            break;
        case 2:
            output = 2;
            break;
        default:
            output = 3;
            break;
    }
    
    return output;
    
case_target:
    /* Safe arithmetic at target */
    output = (code << 1) & 0xFF;  /* Shift and mask - no traps */
    return output;
}

/* Test 6: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit with jump */
        if (sum > 1000) {
            __asm__ volatile("" : : : "memory");
            goto loop_exit;
        }
    }
    
    return sum;
    
loop_exit:
    /* Simple instruction at exit label */
    sum = sum % 256;  /* Modulo with power of 2 - safe */
    return sum;
}

/* Portable fallback for non-delay-slot architectures */
int portable_test(int a, int b) {
    /* Perform similar computations without relying on delay slot behavior */
    int result = a;
    
    if (a > b) {
        result = a + 1;
    } else {
        result = b - 1;
    }
    
    /* Mix in global variables to prevent optimization */
    result += global_counter;
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Delay slot test program\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Seed random values for testing */
    srand(42);
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        checksum += test_unconditional_jump(a, b);
        checksum += test_conditional_jump(b, c);
        checksum += test_multiple_instructions(a);
        checksum += test_nested_jumps(a, b, c);
        checksum += test_switch_jump(a);
        checksum += test_loop_exit(a + 5);
        
        /* Portable version for all architectures */
        checksum += portable_test(a, b);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global a: %d, Global b: %d\n", global_a, global_b);
    
    return checksum != 0 ? 0 : 1;
}
