/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    /* Simple arithmetic that won't trap */
    return x + 1;
}

/* Another non-inlineable function */
__attribute__((noinline, optimize("O0")))
static void dummy_call(int *ptr) {
    *ptr += 2;
}

/* Function with a simple jump to label */
__attribute__((optimize("O2")))
static int test_case_1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use volatile to prevent optimization */
    volatile int *vp = &result;
    
    /* Create a simple goto structure */
    if (a < b) {
        /* This should generate a simple jump */
        goto target_label_1;
    }
    
    /* Dead code to make the jump necessary */
    result = 999;
    return result;
    
target_label_1:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    
    /* Simple arithmetic that doesn't trap */
    c = a + b;
    
    /* Use the result to prevent elimination */
    *vp = c;
    
    return result;
}

/* Test case with function call after label */
__attribute__((optimize("O2")))
static int test_case_2(void) {
    int x = 5;
    int y = 0;
    
    /* Force a simple jump */
    if (x > 0) {
        goto func_call_target;
    }
    
    y = -1;
    return y;
    
func_call_target:
    asm volatile("" ::: "memory");  /* Prevent merging */
    
    /* Function call as delay slot candidate */
    y = simple_operation(x);
    
    return y;
}

/* Test case with asm statement as candidate */
__attribute__((optimize("O2")))
static int test_case_3(void) {
    int var1 = 100, var2 = 200;
    int res = 0;
    
    /* Create jump opportunity */
    if (var1 != 0) {
        goto asm_target;
    }
    
    res = -1;
    return res;
    
asm_target:
    asm volatile("" ::: "memory");  /* Barrier */
    
    /* asm statement that:
       - Modifies only a general purpose register
       - Doesn't reference memory (no "m" constraint)
       - Doesn't trap
       - Sets condition codes (might conflict, but let's try)
    */
    asm volatile("addl %1, %0" 
                 : "+r"(var1) 
                 : "r"(var2)
                 : "cc");
    
    res = var1;
    return res;
}

/* Test case with memory operation (stack variable - safe) */
__attribute__((optimize("O2")))
static int test_case_4(void) {
    int array[4] = {1, 2, 3, 4};
    int sum = 0;
    volatile int *ptr = &sum;
    
    /* Simple jump */
    if (array[0] > 0) {
        goto memory_op;
    }
    
    sum = -1;
    return sum;
    
memory_op:
    asm volatile("" ::: "memory");  /* Barrier */
    
    /* Safe memory access to stack variable */
    sum = array[1] + array[2];
    
    /* Use volatile store to prevent optimization */
    *ptr = sum;
    
    return sum;
}

/* Test case with multiple basic blocks */
__attribute__((optimize("O2")))
static int test_case_5(void) {
    int counter = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        /* Create jump inside loop */
        if (i % 2 == 0) {
            goto loop_target;
        }
        
        counter += i;
        continue;
        
    loop_target:
        asm volatile("" ::: "memory");
        
        /* Simple non-trapping operation */
        counter += 1;
    }
    
    return counter;
}

/* Main function to run all test cases */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_case_4();
    results[5] = test_case_5();
    
    printf("Results: %d %d %d %d %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    
    return 0;
}
