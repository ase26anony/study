/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent function inlining to preserve control flow */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 1;
}

/* Function with asm statement that doesn't conflict with jump resources */
__attribute__((noinline, optimize("O0")))
static int asm_operation(int val) {
    int result = val;
    /* Simple asm that only modifies a general purpose register and doesn't trap */
    asm volatile (
        "addl $5, %0"
        : "+r" (result)
        : 
        : /* No clobbers - avoid condition codes to prevent resource conflicts */
    );
    return result;
}

/* Test function 1: Simple goto with arithmetic after label */
__attribute__((noinline, optimize("O2")))
static int test_goto_with_arithmetic(void) {
    volatile int a = 10;
    volatile int b = 20;
    int result = 0;
    
    /* Use goto to create a simple jump */
    if (a > 5) {
        goto target_label;
    }
    
    /* Some code that won't be executed */
    result = 100;
    return result;
    
target_label:
    /* Candidate for delay slot: simple arithmetic that doesn't trap */
    /* Compiler barrier to prevent merging with label */
    asm volatile("" ::: "memory");
    
    /* Simple operation - good delay slot candidate */
    result = b + a;
    
    /* Use result to prevent elimination */
    return result + 1;
}

/* Test function 2: Nested goto with function call after label */
__attribute__((noinline, optimize("O2")))
static int test_goto_with_function_call(void) {
    volatile int x = 42;
    volatile int y = 13;
    int ret = 0;
    
    /* Create control flow that generates simple jump */
    if (x != 0) {
        goto compute;
    }
    
    /* Dead code path */
    ret = -1;
    return ret;
    
compute:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    ret = simple_operation(x);
    
    /* Use the result */
    return ret + y;
}

/* Test function 3: Loop with goto and asm operation */
__attribute__((noinline, optimize("O1")))
static int test_goto_with_asm(void) {
    int i;
    volatile int sum = 0;
    volatile int counter = 3;
    
    for (i = 0; i < counter; i++) {
        /* Conditional that will generate a simple jump */
        if (i % 2 == 0) {
            goto process_even;
        }
        
        sum += i * 2;
        continue;
        
    process_even:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* asm operation as delay slot candidate */
        sum = asm_operation(sum);
        
        /* Continue loop */
        continue;
    }
    
    return sum;
}

/* Test function 4: Multiple labels with safe operations */
__attribute__((noinline, optimize("O2")))
static int test_multiple_gotos(void) {
    volatile int a = 7, b = 8, c = 9;
    int result = 0;
    
    /* First simple jump */
    if (a > 0) {
        goto label1;
    }
    
    result = -1;
    return result;
    
label1:
    /* First candidate: simple arithmetic */
    asm volatile("" ::: "memory");
    result = a + b;
    
    /* Another jump */
    if (b < 10) {
        goto label2;
    }
    
    return result;
    
label2:
    /* Second candidate: safe function call */
    asm volatile("" ::: "memory");
    result = safe_operation(result, c);
    
    return result;
}

/* Test function 5: Switch-like pattern with goto */
__attribute__((noinline, optimize("O1")))
static int test_switch_goto(void) {
    volatile int option = 2;
    volatile int val1 = 100, val2 = 200;
    int output = 0;
    
    /* Simulate switch with goto */
    if (option == 1) {
        goto case1;
    } else if (option == 2) {
        goto case2;
    } else {
        goto default_case;
    }
    
case1:
    asm volatile("" ::: "memory");
    output = val1 * 2;
    goto end;
    
case2:
    asm volatile("" ::: "memory");
    /* Good delay slot candidate: simple operation */
    output = val2 + 50;
    goto end;
    
default_case:
    asm volatile("" ::: "memory");
    output = 0;
    goto end;
    
end:
    return output;
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test */
    results[0] = test_goto_with_arithmetic();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_goto_with_function_call();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_goto_with_asm();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_multiple_gotos();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_switch_goto();
    printf("Test 5 result: %d\n", results[4]);
    
    /* Use results to prevent optimization */
    int final = 0;
    for (int i = 0; i < 5; i++) {
        final += results[i];
    }
    
    printf("Final sum: %d\n", final);
    return final != 0 ? 0 : 1;
}
