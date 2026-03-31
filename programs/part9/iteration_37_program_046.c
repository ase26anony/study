/* Test program for GCC reorg.cc delay slot optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_func(int x) {
    return x + 1;
}

/* Use optimization barrier to prevent instruction merging */
static void barrier(void) {
    __asm__ volatile("" ::: "memory");
}

/* Test 1: Simple arithmetic after label - good candidate for delay slot */
__attribute__((optimize("O2")))
static int test_simple_arithmetic(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    int result = 0;
    
    /* Create a simple jump to a label */
    if (a > 5) {
        goto target1;
    }
    
    /* Some code to prevent fall-through optimization */
    result = b + c;
    return result;
    
target1:
    /* This instruction should be eligible for delay slot:
       - Simple arithmetic
       - No memory access that could trap
       - Doesn't set resources used by the jump */
    barrier();
    c = a + b;  /* Simple arithmetic - good candidate */
    
    /* Use result to prevent dead code elimination */
    result = c;
    return result;
}

/* Test 2: Function call after label - might be eligible */
__attribute__((optimize("O2")))
static int test_function_call(void) {
    volatile int x = 42;
    volatile int y = 0;
    
    /* Force a simple jump */
    if (x != 0) {
        goto target2;
    }
    
    y = x * 2;
    return y;
    
target2:
    barrier();
    /* Function call - ensure it doesn't conflict with jump resources */
    y = simple_func(x);
    return y;
}

/* Test 3: asm statement with controlled register usage */
__attribute__((optimize("O2")))
static int test_asm_instruction(void) {
    int reg1 = 100;
    int reg2 = 200;
    int reg3 = 300;
    
    /* Create simple jump */
    if (reg1 > 50) {
        goto target3;
    }
    
    reg3 = reg1 + reg2;
    return reg3;
    
target3:
    barrier();
    /* asm that only modifies a specific register, not condition codes
       This avoids resource conflicts with the jump */
    __asm__ volatile(
        "add %1, %0" 
        : "+r" (reg3)      /* Output operand in register */
        : "r" (reg1)       /* Input operand in register */
        /* No clobber list - doesn't modify cc or memory */
    );
    
    return reg3;
}

/* Test 4: Multiple jumps to create more opportunities */
__attribute__((optimize("O2")))
static int test_multiple_jumps(void) {
    volatile int counter = 0;
    volatile int values[4] = {1, 2, 3, 4};
    int sum = 0;
    
    /* First simple jump */
    if (counter == 0) {
        goto first_target;
    }
    
    sum = values[0] + values[1];
    return sum;
    
first_target:
    barrier();
    /* Candidate instruction for first jump's delay slot */
    sum = values[0];
    
    /* Another jump to create more scheduling opportunities */
    if (sum > 0) {
        goto second_target;
    }
    
    sum += values[1];
    return sum;
    
second_target:
    barrier();
    /* Candidate for second jump's delay slot */
    sum += values[2];
    
    return sum;
}

/* Test 5: Avoid trapping instructions */
__attribute__((optimize("O2")))
static int test_no_trap(void) {
    volatile int safe_var = 100;
    volatile int *safe_ptr = &safe_var;  /* Definitely valid pointer */
    int result = 0;
    
    /* Simple jump */
    if (safe_var > 0) {
        goto safe_target;
    }
    
    result = safe_var * 2;
    return result;
    
safe_target:
    barrier();
    /* Safe memory access - stack variable, won't trap */
    result = *safe_ptr;
    
    /* Safe arithmetic - no division that could trap */
    result = result + 1;
    
    return result;
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot optimization patterns...\n");
    
    results[0] = test_simple_arithmetic();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_function_call();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_asm_instruction();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_multiple_jumps();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_no_trap();
    printf("Test 5 result: %d\n", results[4]);
    
    /* Verify results to ensure code wasn't optimized away */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
    }
    printf("Total: %d\n", total);
    
    return 0;
}
