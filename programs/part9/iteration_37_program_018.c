/* test_reorg.c - Program to trigger uncovered delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_calc(int a, int b) {
    return a + b;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void safe_operation(int *ptr) {
    /* Simple operation that shouldn't trap */
    *ptr += 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_case_1(void) {
    volatile int result = 0;
    int a = 10, b = 20;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label;
    }
    
    /* Some code that won't be executed */
    result = -1;
    return result;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    int temp = a + b;
    
    /* Use the result to prevent dead code elimination */
    result = temp;
    
    return result;
}

/* Test case with function call as delay slot candidate */
__attribute__((optimize("O0")))
static int test_case_2(void) {
    volatile int x = 5;
    volatile int y = 3;
    
    /* Create simple jump */
    if (x > 0) {
        goto compute;
    }
    
    return 0;
    
compute:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call that could be moved into delay slot */
    int sum = simple_calc(x, y);
    
    return sum;
}

/* Test case with inline asm as delay slot candidate */
__attribute__((optimize("O0")))
static int test_case_3(void) {
    register int r1 asm("eax") = 100;
    register int r2 asm("ebx") = 200;
    int result = 0;
    
    /* Simple conditional that will always jump */
    if (r1 < r2) {
        goto asm_target;
    }
    
    return -1;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that modifies only specific registers
       and doesn't reference memory or condition codes */
    asm volatile(
        "addl %1, %0\n\t"
        : "+r"(r1)
        : "r"(r2)
        /* No clobbers - don't clobber cc to avoid resource conflicts */
    );
    
    result = r1;
    return result;
}

/* Test case with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static int test_case_4(void) {
    int local_var = 42;  /* Stack variable - safe address */
    int *safe_ptr = &local_var;
    
    /* Always-taken jump */
    if (local_var > 0) {
        goto mem_op;
    }
    
    return 0;
    
mem_op:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory operation on stack variable - shouldn't trap */
    safe_operation(safe_ptr);
    
    return local_var;
}

/* Test case with multiple jumps to create more opportunities */
__attribute__((optimize("O0")))
static int test_case_5(void) {
    int a = 1, b = 2, c = 3, d = 4;
    volatile int results[4] = {0};
    
    /* Series of jumps to different labels */
    if (a) {
        goto label1;
    }
    
label1:
    /* Simple arithmetic - potential delay slot candidate */
    results[0] = a + b;
    
    if (b) {
        goto label2;
    }
    
label2:
    /* Another simple operation */
    results[1] = b + c;
    
    if (c) {
        goto label3;
    }
    
label3:
    /* Final operation */
    results[2] = c + d;
    
    return results[0] + results[1] + results[2];
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
