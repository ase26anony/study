/* test_reorg.c - Test program for GCC reorg pass delay slot optimization */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with O0 optimization to prevent instruction merging */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int result;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label;
    }
    
    /* Some code that won't be executed */
    result = a * b;
    return;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Simple arithmetic operation
       - No memory access that could fault
       - Sets condition codes (cc) but jump is simple unconditional
       - Doesn't conflict with jump's resource set */
    asm volatile("addl $1, %0" : "+r"(a) :: "cc");
    
    /* Use the result to prevent dead code elimination */
    result = a + b;
    printf("Test 1 result: %d\n", result);
}

/* Another test case with function call as candidate */
__attribute__((optimize("O0")))
static void test_case_2(void) {
    volatile int x = 5;
    volatile int y;
    
    /* Create multiple basic blocks to encourage scheduling */
    if (x > 0) {
        goto compute;
    }
    
    y = 0;
    goto end;
    
compute:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    y = simple_operation(x);
    
end:
    printf("Test 2 result: %d\n", y);
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    volatile int array[10] = {0};
    volatile int index = 3;
    volatile int value;
    
    /* Simple jump structure */
    if (index < 10) {
        goto safe_access;
    }
    
    value = -1;
    goto done;
    
safe_access:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory access to stack variable - shouldn't trap */
    value = array[index];
    
    /* Simple arithmetic to use the value */
    value = value * 2 + 1;
    
done:
    printf("Test 3 result: %d\n", value);
}

/* Test with loop structure to create more scheduling opportunities */
__attribute__((optimize("O0")))
static void test_case_4(void) {
    volatile int i;
    volatile int sum = 0;
    
    for (i = 0; i < 10; i++) {
        /* Jump within loop body */
        if (i % 2 == 0) {
            goto even_case;
        }
        
        sum += i * 3;
        continue;
        
    even_case:
        /* Candidate instruction - simple register operation */
        asm volatile("" ::: "memory");
        sum += i * 2;
    }
    
    printf("Test 4 result: %d\n", sum);
}

/* Test with nested jumps */
__attribute__((optimize("O0")))
static void test_case_5(void) {
    volatile int a = 100;
    volatile int b = 200;
    volatile int c;
    
    if (a < b) {
        goto label1;
    }
    
    c = a - b;
    goto final;
    
label1:
    asm volatile("" ::: "memory");
    
    /* Multiple simple operations that could be candidates */
    c = a + b;
    
    if (c > 150) {
        goto label2;
    }
    
    c = c / 2;
    goto final;
    
label2:
    asm volatile("" ::: "memory");
    
    /* Another candidate */
    c = c * 2;
    
final:
    printf("Test 5 result: %d\n", c);
}

int main(void) {
    printf("Testing reorg delay slot optimization...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    return 0;
}
