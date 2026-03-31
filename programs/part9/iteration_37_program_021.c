/* test_reorg.c - Program to trigger uncovered delay slot filling logic in reorg.cc */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline)) static int another_op(int x) {
    return x * 2;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0"))) 
static int test_pattern1(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple jump to a label */
    if (a > 5) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = b + c;
    
target_label1:
    /* Candidate instruction for delay slot - simple arithmetic that doesn't trap */
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Simple arithmetic - won't trap, not a jump, not a sequence */
    c = a + b;
    
    /* Use the result to prevent dead code elimination */
    result += c;
    
    return result;
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O0")))
static int test_pattern2(void) {
    volatile int x = 42;
    int y = 0;
    
    /* Force a simple jump */
    if (x != 0) {
        goto func_call_target;
    }
    
    y = x * 2;
    
func_call_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate - must not be inlined */
    y = simple_operation(x);
    
    return y;
}

/* Test with asm statement as delay slot candidate */
__attribute__((optimize("O0")))
static int test_pattern3(void) {
    int var1 = 100, var2 = 200;
    int sum = 0;
    
    /* Create jump opportunity */
    if (var1 < 150) {
        goto asm_target;
    }
    
    sum = var1 + var2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* asm statement that modifies a register but doesn't reference
       resources that conflict with the jump */
    asm volatile("addl $1, %0" : "+r"(var1) :: /* no clobbers */);
    
    sum += var1;
    return sum;
}

/* Test with memory operation (stack variable - safe) */
__attribute__((optimize("O0")))
static int test_pattern4(void) {
    int array[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    int total = 0;
    
    /* Jump to label */
    if (idx == 0) {
        goto memory_op;
    }
    
    total = array[1] + array[2];
    
memory_op:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory access to stack variable - should not fault */
    array[0] = array[1] + 1;
    
    total += array[0];
    return total;
}

/* Complex pattern with nested jumps */
__attribute__((optimize("O0")))
static int test_pattern5(void) {
    volatile int counter = 0;
    int result = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Create multiple jump opportunities */
        if (i % 2 == 0) {
            goto even_case;
        } else {
            goto odd_case;
        }
        
    even_case:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Simple non-trapping operation */
        result += i * 2;
        continue;
        
    odd_case:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Different simple operation */
        result += i * 3;
        continue;
    }
    
    return result;
}

/* Main function to run all test patterns */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_pattern1();
    printf("Pattern 1 result: %d\n", results[0]);
    
    results[1] = test_pattern2();
    printf("Pattern 2 result: %d\n", results[1]);
    
    results[2] = test_pattern3();
    printf("Pattern 3 result: %d\n", results[2]);
    
    results[3] = test_pattern4();
    printf("Pattern 4 result: %d\n", results[3]);
    
    results[4] = test_pattern5();
    printf("Pattern 5 result: %d\n", results[4]);
    
    /* Verify results to prevent optimization */
    int final = 0;
    for (int i = 0; i < 5; i++) {
        final += results[i];
    }
    
    printf("Final checksum: %d\n", final);
    return final != 0 ? 0 : 1;
}
