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
    
    /* Use inline asm to create a barrier and influence codegen */
    __asm__ volatile ("" : : : "memory");
    
    /* Create an unconditional jump pattern */
    if (x != 0) {
        goto target1;
    }
    
    /* This code should not be reached when x != 0 */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = result + 1;
    result = temp;
    
    /* Additional operations to prevent tail optimization */
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
        /* Force a simple jump to label */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    result = local1 + local2;
    return result;
    
target2:
    /* Candidate instruction: logical operation */
    /* Uses fresh temporary variables */
    int temp1 = local1;
    int temp2 = local2;
    result = temp1 & temp2;  /* Bitwise AND - non-trapping */
    
    global_counter += result;
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int x = val;
    int y = val * 2;
    
    /* Conditional that often evaluates true */
    if (x > 0) {
        __asm__ volatile ("" : : : "memory");
        goto target3;
    }
    
    return x - y;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    /* First instruction is the candidate */
    int tmp = x + 5;      /* Simple addition */
    y = tmp * 2;          /* Follow-up operation */
    
    global_a = x;
    global_b = y;
    return tmp;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int a, int b, int c) {
    int res = a;
    
    /* Outer conditional */
    if (a > b) {
        /* Inner conditional to create more complex flow */
        if (b > c) {
            __asm__ volatile ("" : : : "memory");
            goto target4;
        }
    }
    
    res = b + c;
    return res;
    
target4:
    /* Safe arithmetic with constants only */
    res = res + 1;  /* Increment - very safe operation */
    
    global_counter += res;
    return res;
}

/* Test 5: Function with switch and goto to create jump table */
int test_switch_jump(int code) {
    int value = code;
    
    switch (code & 0x3) {  /* Mask to limit cases */
        case 0:
            value = value * 2;
            break;
        case 1:
            /* This creates a jump to label */
            __asm__ volatile ("" : : : "memory");
            goto target5;
        case 2:
            value = value + 100;
            break;
        default:
            value = value - 50;
            break;
    }
    
    return value;
    
target5:
    /* Very simple instruction - just assignment */
    value = 42;  /* Constant assignment - no resource conflicts */
    
    global_counter += value;
    return value;
}

/* Test 6: Loop with break to label (creates backward jump) */
int test_loop_break(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        
        /* Conditional break to label */
        if (sum > 100) {
            __asm__ volatile ("" : : : "memory");
            goto target6;
        }
    }
    
    return sum;
    
target6:
    /* Simple arithmetic at target */
    sum = sum + 1;
    
    global_a = i;
    return sum;
}

/* Test 7: Use volatile to force register usage patterns */
int test_volatile_registers(int a, int b) {
    volatile int vol_a = a;
    volatile int vol_b = b;
    int result;
    
    if (vol_a != vol_b) {
        __asm__ volatile ("" : : : "memory");
        goto target7;
    }
    
    result = vol_a * vol_b;
    return result;
    
target7:
    /* Use only local temporaries, not volatiles */
    int temp1 = a;
    int temp2 = b;
    result = temp1 - temp2;  /* Subtraction - safe */
    
    global_counter += result;
    return result;
}

/* Test 8: Minimal function focusing on the specific pattern */
#if HAS_DELAY_SLOTS
/* This function is specifically crafted for delay slot architectures */
int test_delay_slot_specific(int x) {
    int local = x;
    
    /* Simple comparison that generates a conditional jump */
    if (local > 0) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* This should generate a simplejump_p in RTL */
        goto delay_target;
    }
    
    return local * 2;
    
delay_target:
    /* IDEAL CANDIDATE FOR DELAY SLOT FILLING:
     * - Simple arithmetic (addition)
     * - Uses local variable not used before jump
     * - No memory access (non-trapping)
     * - Not a jump or call
     * - Not part of SEQUENCE
     */
    local = local + 1;
    
    return local;
}
#else
/* Portable version for non-delay-slot architectures */
int test_delay_slot_specific(int x) {
    return (x > 0) ? (x + 1) : (x * 2);
}
#endif

/* Main driver function */
int main() {
    int total = 0;
    int i;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Seed with some values */
    srand(42);
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        total += test_unconditional_jump(a, b);
        total += test_conditional_jump(a, b);
        total += test_multiple_candidates(a);
        total += test_nested_jumps(a, b, c);
        total += test_switch_jump(a);
        total += test_loop_break(a % 10 + 5);
        total += test_volatile_registers(a, b);
        total += test_delay_slot_specific(a);
        
        /* Prevent loop unrolling */
        __asm__ volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total != 0) ? 0 : 1;
}
