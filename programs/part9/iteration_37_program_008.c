/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int a, int b) {
    return a + b;
}

__attribute__((noinline))
static void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Function with O0 optimization to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_case_1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use goto to create a simple jump */
    if (a > 5) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b;
    
target_label_1:
    /* Compiler barrier to prevent merging with jump */
    memory_barrier();
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Uses registers not conflicting with jump
       - Not a SEQUENCE pattern
    */
    asm volatile("addl %1, %0" 
                 : "+r"(c) 
                 : "r"(a) 
                 : "cc");
    
    /* Use the result to prevent dead code elimination */
    return c + result;
}

/* Another test case with different pattern */
__attribute__((optimize("O0")))
static int test_case_2(void) {
    int x = 100, y = 200;
    volatile int counter = 0;
    
    /* Create multiple basic blocks to encourage reorg */
    if (x < 150) {
        goto compute;
    }
    
    x = 50;
    
compute:
    /* Barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Good delay slot candidate:
       - Register operation only
       - No memory access (won't fault)
       - Sets condition codes but jump is unconditional
    */
    y = x * 2;
    
    /* Function call as potential candidate */
    counter = simple_operation(x, y);
    
    return counter;
}

/* Test with loop to create more scheduling opportunities */
__attribute__((optimize("O1")))  /* Slightly higher optimization for scheduling */
static int test_case_3(void) {
    int i, sum = 0;
    volatile int array[4] = {1, 2, 3, 4};
    
    for (i = 0; i < 3; i++) {
        /* Conditional that will be optimized to simple jump */
        if (array[i] > 0) {
            goto process;
        }
        
        sum += i;
        continue;
        
    process:
        /* Barrier */
        asm volatile("" ::: "memory");
        
        /* Simple instruction that doesn't trap */
        asm volatile("movl %1, %0" 
                     : "=r"(sum) 
                     : "r"(array[i]) 
                     : /* no clobbers */);
        
        /* Another operation to create scheduling opportunity */
        sum += 100;
    }
    
    return sum;
}

/* Test with switch statement that might generate simple jumps */
static int test_case_4(int val) {
    int result = 0;
    
    switch (val) {
        case 1:
            goto case1;
        case 2:
            result = 20;
            break;
        case 3:
            goto case3;
        default:
            return -1;
    }
    
    return result;
    
case1:
    /* Simple arithmetic after label */
    asm volatile("addl $5, %0" : "+r"(result) :: "cc");
    return result;
    
case3:
    /* Load from stack variable (shouldn't fault) */
    int temp = 30;
    asm volatile("movl %1, %0" : "=r"(result) : "r"(temp));
    return result;
}

/* Main orchestrator */
int main(void) {
    int results[4];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test cases */
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_case_4(1);
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
