/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_calc(int a, int b) {
    return a + b;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    /* Simple operation that shouldn't trap */
    return x * 2;
}

/* Function with goto pattern that might trigger delay slot filling */
__attribute__((optimize("O2")))
static int test_goto_pattern1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use volatile to prevent optimization of control flow */
    volatile int flag = 1;
    
    if (flag) {
        /* Simple goto that should generate a simplejump_p */
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b + c;
    
target_label1:
    /* Candidate instruction for delay slot - simple arithmetic */
    /* Compiler barrier to prevent merging with label */
    asm volatile("" ::: "memory");
    
    /* Simple operation that doesn't trap and doesn't conflict with jump */
    /* Using asm to ensure specific instruction pattern */
    int temp = b;
    asm volatile("addl $1, %0" : "+r"(temp) :: "cc");
    result = temp;
    
    return result;
}

/* Another test with different pattern */
__attribute__((optimize("O2")))
static int test_goto_pattern2(void) {
    int x = 5, y = 10, z = 15;
    volatile int flag = 1;
    
    if (flag > 0) {
        goto compute;
    }
    
    /* Dead code to create separation */
    x = y = z = 0;
    
compute:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate - must not be inlined */
    z = simple_calc(x, y);
    
    return z;
}

/* Test with memory operation that shouldn't fault */
__attribute__((optimize("O2")))
static int test_goto_pattern3(void) {
    /* Stack variables are safe - won't cause page faults */
    int array[4] = {1, 2, 3, 4};
    volatile int index = 0;
    int sum = 0;
    
    if (index == 0) {
        goto process;
    }
    
    /* Unreachable but prevents optimization */
    sum = -1;
    
process:
    asm volatile("" ::: "memory");
    
    /* Safe memory access - stack variable */
    sum = array[0] + array[1];
    
    /* Additional operation to ensure instruction is used */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Test with loop and goto */
__attribute__((optimize("O2")))
static int test_goto_pattern4(void) {
    int i, total = 0;
    volatile int limit = 3;
    
    for (i = 0; i < limit; i++) {
        if (i == 1) {
            goto special_case;
        }
        total += i;
        continue;
        
    special_case:
        asm volatile("" ::: "memory");
        /* Simple operation - multiplication by constant */
        total += i * 2;
    }
    
    return total;
}

/* Main orchestrator */
int main(void) {
    int results[4];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_goto_pattern1();
    printf("Pattern 1 result: %d\n", results[0]);
    
    results[1] = test_goto_pattern2();
    printf("Pattern 2 result: %d\n", results[1]);
    
    results[2] = test_goto_pattern3();
    printf("Pattern 3 result: %d\n", results[2]);
    
    results[3] = test_goto_pattern4();
    printf("Pattern 4 result: %d\n", results[3]);
    
    /* Use results to prevent dead code elimination */
    int final = results[0] + results[1] + results[2] + results[3];
    printf("Final sum: %d\n", final);
    
    return final > 0 ? 0 : 1;
}
