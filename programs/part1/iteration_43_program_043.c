/* test_delay_slots.c
 * Designed to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;

/* Function 1: Simple unconditional jump to label with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        /* Force a jump to label */
        goto target1;
    }
    
    /* Some code that won't be reached if x != 0 */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot filling:
     * Simple arithmetic that doesn't conflict with jump resources
     * Using local temporaries not used before the jump
     */
    int temp1 = x + 1;  /* Should be safe to move into delay slot */
    result = temp1;
    
    /* More code after label to ensure it's not optimized away */
    result += y;
    return result;
}

/* Function 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int local1 = a;
    int local2 = b;
    int result = 0;
    
    /* Different comparison to create different jump pattern */
    if (local1 > local2) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    local1 = local1 * 3;
    return local1;
    
target2:
    /* Candidate: Bitwise operation with safe temporaries */
    int temp2 = local1 & 0xFF;  /* Should not trap or conflict */
    result = temp2;
    
    /* Additional computation to prevent optimization */
    result = result | local2;
    return result;
}

/* Function 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int tmp = val;
    int res = 0;
    
    /* Force jump with simple condition */
    if (tmp < 100) {
        goto target3;
    }
    
    /* Unreachable if tmp < 100 */
    tmp = tmp / 2;
    return tmp;
    
target3:
    /* Multiple simple instructions that could be candidates */
    int t1 = tmp + 5;      /* First candidate */
    int t2 = t1 << 2;      /* Second candidate */
    res = t2 ^ 0x55;       /* Third candidate */
    
    /* Use all results to prevent dead code elimination */
    return res + tmp;
}

/* Function 4: Nested jumps to create more complex patterns */
int test_nested_pattern(int x, int y, int z) {
    int a = x, b = y, c = z;
    
    /* First level condition */
    if (a > 0) {
        /* Second level condition */
        if (b < 0) {
            /* This creates a jump to outer label */
            goto outer_target;
        }
        c = c * 2;
    }
    
    return a + b + c;
    
outer_target:
    /* Safe instruction: increment with local temporary */
    int temp = c + 1;
    a = temp;
    
    /* More operations to create valid basic block */
    b = b ^ a;
    return a + b;
}

/* Function 5: Use volatile to prevent certain optimizations */
int test_volatile_jump(int base) {
    volatile int v = base;
    int r = 0;
    
    /* Jump based on volatile to prevent constant folding */
    if (v > 50) {
        __asm__ volatile("" : : : "memory");
        goto volatile_target;
    }
    
    r = base * 3;
    return r;
    
volatile_target:
    /* Very simple instruction - good candidate */
    int safe_temp = base + 2;
    r = safe_temp;
    
    /* Use in computation */
    return r * 2;
}

/* Function 6: Avoid using special registers (like return address) */
int test_safe_registers(int p1, int p2) {
    /* Use only parameter registers and local temps */
    int l1 = p1;
    int l2 = p2;
    
    if (l1 != l2) {
        /* Try to create simple jump */
        goto safe_target;
    }
    
    return l1 - l2;
    
safe_target:
    /* Instruction that only uses local temps set before jump */
    int calc = l1 * l2;  /* Multiplication is usually safe */
    
    /* Avoid division (might trap) */
    if (calc > 0) {
        calc = calc & 0x7FFFFFFF;  /* Safe bitwise op */
    }
    
    return calc;
}

/* Function 7: Mixed operations to test eligibility checks */
int test_mixed_operations(int x) {
    int a = x;
    int b = 0;
    
    /* Multiple conditions to create jump */
    if ((a & 1) == 0) {  /* Even number */
        if (a > 0) {
            goto mix_target;
        }
    }
    
    b = a * 2;
    return b;
    
mix_target:
    /* Various operations that might be eligible */
    int t1 = a + 7;
    int t2 = t1 - 3;
    int t3 = t2 | 0xF;
    
    /* Return computation using all temporaries */
    return t1 + t2 + t3;
}

/* Main driver that calls all test functions */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Call all test functions with different inputs */
    checksum += test_unconditional_jump(5, 3);
    checksum += test_conditional_jump(global_a, global_b);
    checksum += test_multiple_candidates(42);
    checksum += test_nested_pattern(1, -1, 10);
    checksum += test_volatile_jump(60);
    checksum += test_safe_registers(7, 9);
    checksum += test_mixed_operations(24);
    
    /* Add some more calls with different parameters */
    checksum += test_unconditional_jump(0, 8);  /* Different path */
    checksum += test_conditional_jump(global_b, global_a); /* Reversed */
    checksum += test_multiple_candidates(150);  /* Different branch */
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are non-zero to ensure code executed */
    if (checksum == 0) {
        printf("ERROR: All tests returned zero!\n");
        return 1;
    }
    
    return 0;
}
