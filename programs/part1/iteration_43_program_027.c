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
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses temporary variables not live across the jump */
    int temp1 = x + 1;      /* Should not conflict with jump resources */
    int temp2 = y - 1;
    result = temp1 + temp2;
    
    /* Additional instructions to make block non-trivial */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int local1 = a;
    int local2 = b;
    
    /* Create a simple conditional jump */
    if (local1 > local2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    local1 = local2 * 3;
    return local1;
    
target2:
    /* Candidate: bitwise operations (non-trapping) */
    /* Use fresh variables to avoid resource conflicts */
    int fresh1 = local1 & 0xFF;
    int fresh2 = local2 | 0x55;
    int result = fresh1 ^ fresh2;
    
    global_counter += result & 1;
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multi_candidate(int val) {
    int tmp = val;
    
    /* Force jump with goto */
    if (tmp % 2 == 0) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    tmp = tmp * 7;
    return tmp;
    
target3:
    /* Multiple simple instructions that could fill delay slot */
    int t1 = tmp + global_a;    /* Uses global, but safe */
    int t2 = t1 << 2;           /* Shift operation */
    int t3 = t2 ^ 0x1234;       /* XOR with constant */
    
    /* Mix of operations to give compiler choices */
    global_b = t3 % 100;
    return t3;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int a = x, b = y, c = z;
    
    if (a > b) {
        if (b > c) {
            COMPILER_BARRIER();
            goto target4a;
        } else {
            COMPILER_BARRIER();
            goto target4b;
        }
    }
    
    return a + b + c;
    
target4a:
    /* Simple arithmetic candidate */
    a = a * 2;
    b = b + 1;
    return a - b;
    
target4b:
    /* Different arithmetic pattern */
    c = c ^ a;
    a = a | b;
    return c + a;
}

/* Test 5: Function with switch and goto to create jump table */
int test_switch_jump(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            COMPILER_BARRIER();
            goto target5_0;
        case 1:
            result = code * 2;
            break;
        case 2:
            COMPILER_BARRIER();
            goto target5_2;
        default:
            result = code + 100;
            break;
    }
    
    return result;
    
target5_0:
    /* Safe arithmetic with constants */
    result = (code + 5) * 3;
    global_counter += 1;
    return result;
    
target5_2:
    /* Logical operations only */
    result = (code & 0xF) | ((code >> 4) & 0xF0);
    return result;
}

/* Test 6: Loop with internal jump (more complex control flow) */
int test_loop_jump(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        if (i == iterations / 2) {
            COMPILER_BARRIER();
            goto loop_target;
        }
        sum += i;
    }
    
    return sum;
    
loop_target:
    /* Instruction at jump target inside loop */
    sum = sum * 2 + 1;
    
    /* Continue loop */
    for (; i < iterations; i++) {
        sum += i * 3;
    }
    
    return sum;
}

/* Test 7: Use inline assembly to hint at delay slot opportunities */
#if HAS_DELAY_SLOTS
int test_asm_hint(int a, int b) {
    int res;
    
    /* Force a branch instruction */
    if (a > b) {
        /* Inline asm to prevent certain optimizations */
        __asm__ volatile(
            "nop \n\t"
            : : : "memory"
        );
        goto asm_target;
    }
    
    res = b - a;
    return res;
    
asm_target:
    /* Very simple instruction ideal for delay slot */
    res = a + b;
    
    /* Another simple instruction */
    global_a = res & 0xFF;
    
    return res;
}
#else
int test_asm_hint(int a, int b) {
    /* Portable version */
    return (a > b) ? (a + b) : (b - a);
}
#endif

/* Test 8: Avoid using return address register in target instruction */
int test_no_ra_conflict(int x) {
    int local = x;
    
    /* Jump that's not a return */
    if (local != 0) {
        COMPILER_BARRIER();
        goto no_ra_target;
    }
    
    return local * 2;
    
no_ra_target:
    /* Use only temporary registers (not $ra on MIPS) */
    int t1 = local + 7;
    int t2 = t1 * 3;
    
    /* Avoid function calls that might use $ra */
    return t2 - 5;
}

/* Main driver that exercises all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing delay slot filling (HAS_DELAY_SLOTS = %d)\n", HAS_DELAY_SLOTS);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i * 2);
        checksum += test_conditional_jump(i, i + 5);
        checksum += test_multi_candidate(i * 3);
        checksum += test_nested_jumps(i, i + 1, i + 2);
        checksum += test_switch_jump(i);
        checksum += test_loop_jump(5 + (i % 3));
        checksum += test_asm_hint(i, 10 - i);
        checksum += test_no_ra_conflict(i * 7);
        
        /* Prevent loop unrolling from eliminating jumps */
        COMPILER_BARRIER();
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global b: %d\n", global_b);
    
    return checksum != 0 ? 0 : 1;
}
