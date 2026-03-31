/* Test program for GCC reorg.cc delay slot filling optimization
 * Targeting lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another noinline function to use as delay slot candidate */
__attribute__((noinline))
static int safe_operation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 1;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_pattern1(void) {
    int a = 10, b = 20, c = 0;
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Create a simple jump to a label */
    if (a > 5) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents dead code elimination */
    result = a + b;
    
target_label1:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - No memory access that could fault
       - Doesn't create a SEQUENCE pattern
    */
    c = a + b;
    
    /* Use the result to prevent elimination */
    result = c;
    printf("Pattern1 result: %d\n", result);
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O0")))
static void test_pattern2(void) {
    int x = 5, y = 0;
    volatile int res = 0;
    
    /* Force a simple jump */
    if (x != 0) {
        goto func_target;
    }
    
    y = x * 2;
    
func_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as candidate - must not conflict with jump resources */
    y = simple_operation(x);
    
    res = y;
    printf("Pattern2 result: %d\n", res);
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static void test_pattern3(void) {
    int var1 = 100, var2 = 200;
    volatile int output = 0;
    
    /* Create multiple basic blocks to encourage scheduling */
    for (int i = 0; i < 3; i++) {
        if (var1 > 50) {
            goto asm_target;
        }
        var1++;
    }
    
    output = var1 + var2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* asm statement that:
       - Only modifies a general purpose register
       - Doesn't touch condition codes (no "cc" clobber)
       - No memory operands
    */
    asm volatile("addl %1, %0" 
                 : "+r"(var1) 
                 : "r"(var2)
                 : /* no clobbers */);
    
    output = var1;
    printf("Pattern3 result: %d\n", output);
}

/* More complex pattern with nested jumps */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static void test_pattern4(void) {
    int counter = 0;
    int values[4] = {1, 2, 3, 4};
    volatile int sum = 0;
    
    /* Loop with internal jump to create scheduling opportunities */
    for (int i = 0; i < 4; i++) {
        if (values[i] > 2) {
            goto process_value;
        }
        counter++;
        continue;
        
    process_value:
        /* Barrier to prevent sequence formation */
        asm volatile("" ::: "memory");
        
        /* Safe operation - multiplication won't trap for these values */
        int temp = safe_operation(values[i], counter);
        
        sum += temp;
        counter = 0;
    }
    
    printf("Pattern4 sum: %d\n", sum);
}

/* Test that specifically avoids trapping instructions */
__attribute__((optimize("O0")))
static void test_pattern5(void) {
    int a = 1, b = 2, c = 3, d = 4;
    volatile int r1 = 0, r2 = 0;
    
    /* Multiple jumps to same label */
    if (a < b) {
        goto compute1;
    }
    
    if (c < d) {
        goto compute2;
    }
    
compute1:
    asm volatile("" ::: "memory");
    /* Bitwise operations never trap */
    r1 = a & b;
    goto end;
    
compute2:
    asm volatile("" ::: "memory");
    /* Shift operations are safe */
    r2 = c << 1;
    
end:
    printf("Pattern5 results: %d, %d\n", r1, r2);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    return 0;
}
