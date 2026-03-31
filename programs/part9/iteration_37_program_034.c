/* Test program for GCC reorg.cc delay slot filling optimization
 * Targeting lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void safe_operation(int *ptr) {
    *ptr += 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    int result = 0;
    
    /* Create a simple jump to a label */
    if (a > 5) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = b + c;
    
target_label_1:
    /* Candidate instruction for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic that doesn't trap
       - No resource conflicts with the jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    c = b + 5;  /* Simple arithmetic - won't trap */
    
    /* Use the result to prevent dead code elimination */
    result += c;
    return result;
}

/* Test case with function call as delay slot candidate */
__attribute__((optimize("O0")))
static int test_case_2(void) {
    volatile int x = 100;
    volatile int y = 200;
    int result = 0;
    
    /* Force a simple jump */
    if (x < 1000) {
        goto target_label_2;
    }
    
    result = y * 2;
    
target_label_2:
    asm volatile("" ::: "memory");  /* Prevent merging with label */
    
    /* Function call that doesn't throw and has no resource conflicts */
    result = simple_operation(x);
    
    return result;
}

/* Test case with asm statement as delay slot candidate */
__attribute__((optimize("O0")))
static int test_case_3(void) {
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    
    /* Create conditions for a simple jump */
    if (var1 > 0) {
        goto target_label_3;
    }
    
    var3 = var1 + var2;
    
target_label_3:
    asm volatile("" ::: "memory");  /* Barrier to prevent SEQUENCE formation */
    
    /* Inline asm that modifies only a specific register
       This should not conflict with jump resources */
    asm volatile(
        "addl $1, %0\n\t"
        : "+r"(var2)  /* Only modifies var2 through register */
        : 
        : /* No clobbers - avoids cc which might conflict */
    );
    
    return var1 + var2 + var3;
}

/* Test case with memory operation (safe stack access) */
__attribute__((optimize("O0")))
static int test_case_4(void) {
    int array[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    int sum = 0;
    
    /* Simple jump condition */
    if (idx == 0) {
        goto target_label_4;
    }
    
    sum = array[1] + array[2];
    
target_label_4:
    asm volatile("" ::: "memory");
    
    /* Safe memory access to stack variable - won't fault */
    array[0] = array[1] + 1;
    
    /* Use result */
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    return sum;
}

/* Test case that avoids trapping instructions */
__attribute__((optimize("O0")))
static int test_case_5(void) {
    volatile int p = 1;
    volatile int q = 2;
    int r = 3;
    
    /* Multiple jumps to create more opportunities */
    if (p > 0) {
        goto target_1;
    }
    
    r = p + q;
    
target_1:
    asm volatile("" ::: "memory");
    /* Simple logical operation - guaranteed not to trap */
    r = p & q;
    
    if (r > 0) {
        goto target_2;
    }
    
    p = q + r;
    
target_2:
    asm volatile("" ::: "memory");
    /* Bit manipulation - safe */
    r = p | 0x01;
    
    return r;
}

/* Main function to run all test cases */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_case_4();
    results[4] = test_case_5();
    
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
