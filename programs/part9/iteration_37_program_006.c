/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline))
static int safe_computation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 1;
}

/* Function with a simple jump to label pattern */
__attribute__((noinline, optimize("O0")))
static int test_jump_to_label(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use volatile to prevent optimization of control flow */
    volatile int trigger = 1;
    
    if (trigger) {
        /* This should generate a simple jump to label */
        goto target_label;
    }
    
    /* Dead code that won't be executed but prevents optimization */
    a = b + c;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Uses local variables (stack-based, safe)
       - No complex addressing modes
    */
    result = a + b;
    
    /* Use the result to prevent dead code elimination */
    return result;
}

/* Test with inline asm as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int test_asm_candidate(void) {
    int x = 5, y = 3;
    volatile int trigger = 1;
    
    if (trigger) {
        goto asm_target;
    }
    
    /* Dead code */
    x = y * 2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that should be eligible for delay slot:
       - Only modifies general purpose register (eax)
       - Doesn't touch memory or condition codes
       - Simple operation that won't trap
    */
    asm volatile("addl $1, %0" : "+r"(x) ::);
    
    return x;
}

/* Test with function call as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int test_function_call_candidate(void) {
    int val = 100;
    volatile int trigger = 1;
    
    if (trigger) {
        goto func_target;
    }
    
    val = 0;
    
func_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call that doesn't trap and has no resource conflicts
       with the jump instruction */
    val = simple_operation(val);
    
    return val;
}

/* Test with multiple basic blocks to encourage reorg optimization */
__attribute__((noinline))
static int test_complex_pattern(void) {
    int i, sum = 0;
    volatile int array[10];
    
    /* Initialize array */
    for (i = 0; i < 10; i++) {
        array[i] = i;
    }
    
    /* Create a pattern with jumps and labels */
    for (i = 0; i < 10; i++) {
        if (array[i] > 5) {
            /* This should generate a simple jump */
            goto process_large;
        }
        
        sum += array[i];
        continue;
        
    process_large:
        /* Candidate instruction: safe computation */
        sum += safe_computation(array[i], i);
    }
    
    return sum;
}

/* Test to avoid sequence formation */
__attribute__((noinline, optimize("O0")))
static int test_avoid_sequence(void) {
    int x = 1, y = 2, z = 3;
    volatile int trigger = 1;
    
    if (trigger) {
        goto no_sequence_target;
    }
    
    x = y + z;
    
no_sequence_target:
    /* Multiple compiler barriers to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Simple instruction that won't be expanded to SEQUENCE */
    x = x & 0xFF;  /* Simple mask operation */
    
    asm volatile("" ::: "memory");
    
    /* Another simple operation */
    y = z - 1;
    
    return x + y;
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test pattern */
    results[0] = test_jump_to_label();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_asm_candidate();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_function_call_candidate();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_complex_pattern();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_avoid_sequence();
    printf("Test 5 result: %d\n", results[4]);
    
    /* Verify all tests produced expected results */
    if (results[0] == 30 &&  /* 10 + 20 */
        results[1] == 6 &&   /* 5 + 1 */
        results[2] == 101 && /* 100 + 1 */
        results[4] == 4) {   /* (1 & 0xFF) + (3 - 1) = 1 + 2 = 3 */
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed\n");
        return 1;
    }
}
