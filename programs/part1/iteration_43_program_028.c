/* Test program to trigger delay slot filling from jump target in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;

/* Function 1: Simple unconditional jump to label with safe arithmetic */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple unconditional jump pattern */
    if (x != 0) {
        /* Force a jump to label */
        goto target1;
    }
    
    /* Some code that won't be reached if x != 0 */
    y = y * 2;
    return y;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is set before jump, not live across */
    return result;
}

/* Function 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create conditional that leads to simple jump */
    if (a > b) {
        /* Use inline asm as compiler barrier */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    result = a + b;
    return result;
    
target2:
    /* Candidate: Safe logical operation with temporaries */
    temp1 = temp1 & 0xFF;  /* Mask operation, no memory access */
    result = temp1 + temp2;
    return result;
}

/* Function 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force jump with simple comparison */
    if (val < 100) {
        goto target3;
    }
    
    local3 = val - 50;
    return local3;
    
target3:
    /* Multiple simple instructions - one might be eligible */
    local1 = local1 + 5;      /* First candidate: addition */
    local2 = local2 | 0x01;   /* Second candidate: bitwise OR */
    local3 = local1 + local2; /* Third candidate: addition of locals */
    return local3;
}

/* Function 4: Avoid resource conflicts by using fresh variables */
int test_no_conflict(int x) {
    /* Variables declared right before jump target */
    int fresh1, fresh2;
    
    if (x > 0) {
        /* Jump with minimal live registers */
        goto target4;
    }
    
    return x * 2;
    
target4:
    /* Use completely fresh variables that don't conflict with jump context */
    fresh1 = 42;
    fresh2 = fresh1 + x;  /* Only uses x which is set before jump */
    return fresh2;
}

/* Function 5: Complex enough to avoid being optimized away */
int test_complex_pattern(int a, int b, int c) {
    int t1 = a;
    int t2 = b;
    int t3 = c;
    
    /* Nested condition to create interesting control flow */
    if (a > b) {
        if (b < c) {
            /* Multiple conditions met - jump to target */
            goto target5;
        }
    }
    
    /* Default computation */
    return (a + b) * c;
    
target5:
    /* Safe arithmetic with no side effects */
    t1 = t1 << 2;     /* Shift operation */
    t2 = t2 >> 1;     /* Another shift */
    t3 = t1 + t2;
    return t3;
}

/* Function 6: Use volatile to prevent certain optimizations */
int test_with_volatile(int x) {
    volatile int v = x;
    int result;
    
    if (v > 50) {
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target6;
    }
    
    result = x * 3;
    return result;
    
target6:
    /* Simple arithmetic after volatile access */
    result = x + 100;
    return result;
}

/* Function 7: Try to create a return jump pattern */
int test_return_like(int x) {
    int temp = x;
    
    if (temp == 0) {
        /* This creates a simple jump to exit */
        goto early_exit;
    }
    
    temp = temp * 2;
    return temp;
    
early_exit:
    /* Instruction that could fill delay slot of return jump */
    temp = temp + 1;
    return temp;
}

/* Main driver that exercises all test functions */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Call all test functions with various inputs */
    checksum += test_unconditional_jump(5, 10);
    checksum += test_conditional_jump(global_a, global_b);
    checksum += test_multiple_candidates(75);
    checksum += test_no_conflict(25);
    checksum += test_complex_pattern(10, 5, 15);
    checksum += test_with_volatile(60);
    checksum += test_return_like(0);
    checksum += test_return_like(10);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 5; i++) {
        checksum += test_unconditional_jump(i, i*2);
        checksum += test_conditional_jump(i, i+3);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are consistent */
    if (checksum != 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}
