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
    result = result + 1;  /* Should use a register not live across jump */
    
    /* Additional instructions to prevent tail optimization */
    global_counter++;
    return result + global_counter;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Use volatile to force real comparison */
    if (global_a > global_b) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Candidate: Bitwise operation with temporaries */
    temp1 = temp1 ^ 0xFF;  /* Should be safe to move into delay slot */
    result = temp1 + temp2;
    
    global_counter += 2;
    return result + (global_counter & 0xF);
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multi_candidate(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force a jump */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    local3 = local1 * local2;
    return local3;
    
target3:
    /* Multiple simple instructions - one should be eligible */
    local1 = local1 | 0x01;      /* Simple bitwise OR */
    local2 = local2 + local1;    /* Addition with local var */
    local3 = local2 >> 1;        /* Shift operation */
    
    /* Use result to prevent dead code elimination */
    global_counter += local3;
    return local1 + local2 + local3;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x) {
    int a = x;
    int b = x + 1;
    
    /* First level condition */
    if (a > 0) {
        /* Second level condition */
        if (b < 50) {
            COMPILER_BARRIER();
            goto target4;
        }
        a = a * 2;
    }
    
    b = b - a;
    return b;
    
target4:
    /* Simple arithmetic candidate */
    a = a + b;          /* Uses two local variables */
    b = b & ~0x1;       /* Clear LSB */
    
    global_counter += a;
    return a ^ b;
}

/* Test 5: Function with switch and goto to create jump table */
int test_switch_jump(int code) {
    int result = code;
    
    switch (code & 0x3) {
        case 0:
            result = result * 2;
            break;
        case 1:
            COMPILER_BARRIER();
            goto target5;  /* Jump from switch case */
            break;
        case 2:
            result = result / 2;
            break;
        default:
            result = result + 100;
            break;
    }
    
    return result;
    
target5:
    /* Candidate instruction after switch jump */
    result = result << 1;  /* Left shift */
    result = result | 0x1; /* Set LSB */
    
    global_counter += result;
    return result;
}

/* Test 6: Loop with conditional exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Early exit with jump */
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto target6;
        }
    }
    
    return sum;
    
target6:
    /* Arithmetic at jump target */
    sum = sum & 0x3FF;  /* Mask to 10 bits */
    i = i * 2;
    
    global_counter += i;
    return sum + i;
}

/* Test 7: Use inline assembly to hint at delay slot filling (MIPS/SPARC specific) */
#if HAS_DELAY_SLOTS
int test_asm_hint(int x, int y) {
    int result = x;
    
    /* Force a branch instruction */
    if (x > y) {
        /* Inline asm to prevent optimization */
        __asm__ volatile(
            ".set noreorder\n\t"
            ".set nomacro\n\t"
            "nop\n\t"
            ".set macro\n\t"
            ".set reorder\n\t"
            : : : "memory"
        );
        goto target7;
    }
    
    result = y - x;
    return result;
    
target7:
    /* Very simple instruction that should be delay-slot eligible */
    result = result + 5;
    
    /* Use different register classes if possible */
    volatile int temp = result;
    result = temp * 2;
    
    global_counter += 3;
    return result;
}
#else
/* Fallback for non-delay-slot architectures */
int test_asm_hint(int x, int y) {
    return x + y + 7;
}
#endif

/* Test 8: Avoid resource conflicts by using fresh variables */
int test_no_conflict(int base) {
    /* Variables declared right before jump - less likely to be in 'needed' set */
    int fresh1 = base;
    int fresh2 = base + 1;
    
    if (fresh1 != fresh2) {
        COMPILER_BARRIER();
        goto target8;
    }
    
    return fresh1 * fresh2;
    
target8:
    /* Use only the fresh variables - shouldn't conflict with jump resources */
    fresh1 = fresh1 + fresh2;
    fresh2 = fresh2 * 3;
    
    /* Store to global to prevent optimization */
    volatile int store = fresh1;
    (void)store;
    
    return fresh1 - fresh2;
}

/* Main driver that runs all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 3; i++) {
        checksum += test_unconditional_jump(i * 10, i * 5);
        checksum += test_conditional_jump(i + 1, i + 2);
        checksum += test_multi_candidate(i * 7);
        checksum += test_nested_jumps(i * 3);
        checksum += test_switch_jump(i);
        checksum += test_loop_exit(20 + i);
        checksum += test_asm_hint(i * 2, i * 3);
        checksum += test_no_conflict(i * 11);
        
        /* Modify globals to change behavior */
        global_a += i;
        global_b -= i;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0) ? 0 : 1;
}
