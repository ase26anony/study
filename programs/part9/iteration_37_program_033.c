/* Test program to trigger delay slot filling logic in GCC's reorg pass */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c -o test.o */
/* Or for x86: gcc -O3 -m32 -fno-gcse -fno-crossjumping -c test.c -o test.o */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to maintain control flow structure */
__attribute__((noinline, optimize("O2")))
static int simple_arithmetic(int a, int b) {
    return a + b;
}

/* Function with a simple jump to label with candidate instruction */
__attribute__((noinline, optimize("O2")))
static int test_case_1(int x) {
    int result = 0;
    
    /* Use volatile to prevent optimization */
    volatile int trigger = x;
    
    if (trigger > 100) {
        /* This should generate a simple jump */
        goto target_label;
    }
    
    /* Some code to make the basic block non-trivial */
    result = x * 2;
    
target_label:
    /* Candidate instruction for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic (shouldn't trap)
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    
    /* Simple arithmetic that doesn't trap */
    int temp = x + 1;
    
    /* Use the result to prevent dead code elimination */
    result += temp;
    
    return result;
}

/* Test case with function call as candidate */
__attribute__((noinline, optimize("O2")))
static int test_case_2(int x) {
    int result = 0;
    volatile int condition = x;
    
    if (condition > 50) {
        goto jump_target;
    }
    
    result = x - 10;
    
jump_target:
    /* Compiler barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    result = simple_arithmetic(result, x);
    
    return result;
}

/* Test with memory operation (stack variable - safe) */
__attribute__((noinline, optimize("O2")))
static int test_case_3(void) {
    int array[4] = {1, 2, 3, 4};
    int sum = 0;
    volatile int flag = 1;
    
    if (flag) {
        goto compute;
    }
    
    sum = 100;
    
compute:
    /* Barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Safe memory access to stack variable */
    int val = array[2];  /* Index 2 is within bounds */
    
    /* Simple operation that doesn't trap */
    sum += val * 2;
    
    return sum;
}

/* Test with asm statement as candidate */
__attribute__((noinline, optimize("O1")))  /* O1 to keep things simple */
static int test_case_4(int x) {
    int result = x;
    volatile int check = x % 3;
    
    if (check == 0) {
        goto asm_target;
    }
    
    result = x * 3;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that modifies a register but doesn't conflict */
    int temp = result;
    asm volatile(
        "addl $5, %0\n\t"
        : "+r"(temp)
        :
        : "cc"  /* Condition codes might conflict, but let's try */
    );
    
    return temp + 1;
}

/* Nested jumps to create more complex patterns */
__attribute__((noinline, optimize("O2")))
static int test_case_5(int x) {
    int a = x;
    int b = 0;
    volatile int v1 = x > 10;
    volatile int v2 = x < 100;
    
    if (v1) {
        if (v2) {
            goto inner_label;
        }
        a = x * 2;
    }
    
    b = a + 5;
    return b;
    
inner_label:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Simple non-trapping operation */
    b = a | 0xFF;
    
    return b;
}

/* Main function to run all test cases */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test case multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        results[0] = test_case_1(i * 20);
        results[1] = test_case_2(i * 10);
        results[2] = test_case_3();
        results[3] = test_case_4(i);
        results[4] = test_case_5(i);
        
        /* Use results to prevent optimization */
        printf("Iteration %d: %d %d %d %d %d\n", 
               i, results[0], results[1], results[2], results[3], results[4]);
    }
    
    return 0;
}
