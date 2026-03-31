/* test_reorg.c - Program to trigger delay slot filling optimization in GCC's reorg pass */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void safe_arithmetic(int *a, int *b) {
    *a = *b + 5;
}

/* Function with specific optimization level to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_case_1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use goto to create a simple jump instruction */
    if (a < b) {
        goto target_label_1;
    }
    
    /* Dead code that won't be executed but prevents optimization */
    result = a + b + c;
    
target_label_1:
    /* Candidate instruction for delay slot:
       - Simple arithmetic operation
       - No memory access that could fault
       - No function call that could throw
       - Doesn't conflict with jump resources
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    c = a + b;  /* Simple arithmetic - good candidate */
    asm volatile("" ::: "memory");  /* Prevent merging */
    
    return c + result;
}

/* Test case with function call after label */
__attribute__((optimize("O1")))
static int test_case_2(void) {
    int x = 5, y = 10;
    volatile int guard = 0;
    
    /* Create simple jump pattern */
    if (guard == 0) {
        goto compute_label;
    }
    
    /* Unreachable code to create separation */
    x = y * 2;
    
compute_label:
    /* Function call as delay slot candidate */
    asm volatile("" ::: "memory");
    x = simple_operation(y);  /* Function call - must not conflict with jump */
    asm volatile("" ::: "memory");
    
    return x;
}

/* Test case with inline asm as delay slot candidate */
__attribute__((optimize("O2")))
static int test_case_3(void) {
    int reg1 = 100, reg2 = 200;
    int condition = 1;
    
    /* Simple conditional that will always jump */
    if (condition) {
        goto asm_target;
    }
    
    reg1 = reg2 * 3;  /* Never executed */
    
asm_target:
    /* Inline asm instruction as delay slot candidate:
       - Only modifies specific register
       - No memory access
       - No condition code clobber if possible
    */
    asm volatile("" ::: "memory");
    /* Simple register operation - modify reg1 using reg2 */
    asm volatile("addl %1, %0" : "+r"(reg1) : "r"(reg2));
    asm volatile("" ::: "memory");
    
    return reg1;
}

/* Test case with memory operation (safe stack access) */
__attribute__((optimize("O0")))
static int test_case_4(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 0;
    int temp = 0;
    
    /* Always-taken jump */
    if (array[0] > 0) {
        goto memory_op;
    }
    
    temp = 999;  /* Dead code */
    
memory_op:
    /* Safe memory access to stack variable - won't fault */
    asm volatile("" ::: "memory");
    temp = array[index];  /* Safe array access with known index */
    asm volatile("" ::: "memory");
    
    return temp;
}

/* Complex test with nested control flow */
static int test_case_5(int iterations) {
    int sum = 0;
    int i = 0;
    
    for (i = 0; i < iterations; i++) {
        int local_var = i * 2;
        
        /* Inner conditional with goto */
        if (local_var % 3 == 0) {
            goto process;
        }
        
        /* Alternative path */
        sum += local_var;
        continue;
        
    process:
        /* Delay slot candidate in loop */
        asm volatile("" ::: "memory");
        sum += local_var + 1;  /* Simple arithmetic */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Main function to execute all test cases */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute each test case */
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_case_4();
    results[4] = test_case_5(10);
    
    /* Use results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
        printf("Test %d: %d\n", i + 1, results[i]);
    }
    
    printf("Total: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
