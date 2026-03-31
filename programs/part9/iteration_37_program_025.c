/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_computation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 1;
}

/* Function with compiler barrier to prevent instruction merging */
__attribute__((noinline, optimize("O0")))
static void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test case 1: Simple goto with arithmetic after label */
__attribute__((optimize("O0")))  /* Prevent early optimizations */
static int test_case_1(int x) {
    int result = 0;
    
    if (x > 100) {
        goto target_label;
    }
    
    /* Some code to make the basic block non-trivial */
    result = x * 2;
    
    /* This is the simple jump that should become simplejump_p */
    if (result > 50) {
        goto target_label;
    }
    
    return result;
    
target_label:
    /* Compiler barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Candidate for delay slot: simple arithmetic, no memory access */
    /* Use asm to control exact instruction generation */
    int temp = x;
    asm volatile("addl $1, %0" : "+r"(temp) :: "cc");
    
    /* Use the result to prevent elimination */
    return temp + 10;
}

/* Test case 2: Nested jumps with function call candidate */
__attribute__((optimize("O0")))
static int test_case_2(int x) {
    volatile int counter = 0;  /* volatile to prevent optimization */
    
    /* Multiple conditions to create jump structure */
    if (x < 0) {
        goto skip;
    }
    
    if (x > 1000) {
        goto skip;
    }
    
    counter = 1;
    goto compute;
    
skip:
    counter = -1;
    
compute:
    /* This jump should be simplejump_p */
    if (counter == 1) {
        goto process;
    }
    
    return counter;
    
process:
    /* Barrier to prevent SEQUENCE formation */
    memory_barrier();
    
    /* Function call as delay slot candidate - must not conflict with jump */
    int y = simple_operation(x);
    
    /* Use result */
    return y * 2;
}

/* Test case 3: Loop with exit jump */
__attribute__((optimize("O0")))
static int test_case_3(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        if (i == 5) {
            /* This should generate a simple jump */
            goto special_case;
        }
        sum += i;
    }
    
    return sum;
    
special_case:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Safe computation that won't trap */
    int temp = safe_computation(n, i);
    
    /* Simple asm that doesn't reference memory */
    asm volatile("movl %1, %0" : "=r"(sum) : "r"(temp));
    
    return sum;
}

/* Test case 4: Switch-like structure with goto */
__attribute__((optimize("O0")))
static int test_case_4(int option) {
    int result = 0;
    
    switch (option) {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            /* Fall through to default via goto */
            goto default_case;
        default:
default_case:
            /* Barrier */
            asm volatile("" ::: "memory");
            
            /* Simple arithmetic - good delay slot candidate */
            result = result * 2 + 1;
            
            /* Another asm that only modifies a register */
            int reg = result;
            asm volatile("andl $0xFF, %0" : "+r"(reg));
            result = reg;
            break;
    }
    
    return result;
}

/* Test case 5: Multiple labels and jumps */
__attribute__((optimize("O0")))
static int test_case_5(int x) {
    static int data[10] = {0};  /* Static to ensure safe memory access */
    
    if (x < 0) goto negative;
    if (x == 0) goto zero;
    
    /* Main path */
    data[0] = x;
    goto finish;
    
negative:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Safe memory access to stack variable */
    int local = x;
    local = -local;  /* Simple arithmetic */
    data[1] = local;
    goto finish;
    
zero:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Register-only operations */
    int a = 0, b = 0;
    asm volatile("movl $100, %0" : "=r"(a));
    asm volatile("addl $50, %0" : "+r"(b));
    data[2] = a + b;
    /* Fall through */
    
finish:
    return data[0] + data[1] + data[2];
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test case */
    results[0] = test_case_1(42);
    results[1] = test_case_2(500);
    results[2] = test_case_3(10);
    results[3] = test_case_4(2);
    results[4] = test_case_5(5);
    
    /* Print results to prevent optimization */
    for (int i = 0; i < 5; i++) {
        printf("Test %d: %d\n", i + 1, results[i]);
    }
    
    return 0;
}
