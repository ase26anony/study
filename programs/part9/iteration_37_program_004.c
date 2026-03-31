/* Test program for reorg.cc delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function */
__attribute__((noinline))
static int another_op(int x) {
    return x * 2;
}

/* Function with a simple jump pattern that should trigger delay slot filling */
__attribute__((optimize("O2")))
static int test_delay_slot_fill(int x) {
    int a = x;
    int b = 0;
    int c = 0;
    
    /* Use volatile to prevent optimization */
    volatile int v = 0;
    
    /* Create a simple goto that will become a simplejump_p */
    if (x > 100) {
        goto target_label;
    }
    
    /* Some code to make the basic block non-trivial */
    a = x * x;
    b = x + 5;
    
    /* This is the target label with a candidate instruction */
target_label:
    /* Compiler barrier to prevent merging with previous instruction */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Uses registers (not memory that might fault)
       - Doesn't create a SEQUENCE pattern
    */
    c = a + b + v;
    
    /* Use the result to prevent dead code elimination */
    return c;
}

/* Test with function call as candidate */
__attribute__((optimize("O2")))
static int test_function_call_candidate(int x) {
    int result = x;
    
    /* Simple jump to label */
    if (x % 2 == 0) {
        goto func_target;
    }
    
    result = x * 3;
    
func_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as candidate - must not be inlined */
    result = simple_operation(result);
    
    return result;
}

/* Test with asm statement as candidate */
__attribute__((optimize("O2")))
static int test_asm_candidate(int x) {
    int a = x;
    int b = 0;
    
    /* Create condition for simple jump */
    if (x > 50) {
        goto asm_target;
    }
    
    a = x * 2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* ASM instruction as candidate:
       - Simple register operation
       - No memory reference (won't fault)
       - Modifies specific register
       - Sets condition codes (might conflict, but let's test)
    */
    asm volatile(
        "addl $1, %0\n\t"
        : "+r"(a)
        :
        : "cc"
    );
    
    return a + b;
}

/* Test with memory operation (stack variable - safe) */
__attribute__((optimize("O2")))
static int test_memory_candidate(int x) {
    int array[4] = {x, x+1, x+2, x+3};
    int sum = 0;
    volatile int trigger = 1;
    
    if (trigger) {
        goto mem_target;
    }
    
    sum = x * 10;
    
mem_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory access to stack variable - should be safe */
    sum = array[1] + array[2];
    
    return sum;
}

/* Complex test with multiple jumps */
__attribute__((optimize("O2")))
static int test_multiple_jumps(int x) {
    int r1 = x;
    int r2 = x * 2;
    int r3 = 0;
    
    /* First simple jump */
    if (x > 1000) {
        goto target1;
    }
    
    r1 = another_op(r1);
    
target1:
    asm volatile("" ::: "memory");
    /* Candidate instruction 1 */
    r3 = r1 + r2;
    
    /* Second simple jump */
    if (x < 0) {
        goto target2;
    }
    
    r2 = r1 * 3;
    
target2:
    asm volatile("" ::: "memory");
    /* Candidate instruction 2 */
    r3 = r2 - r1;
    
    return r3;
}

/* Main function to run all tests */
int main(int argc, char **argv) {
    int test_val = 42;
    int result = 0;
    
    printf("Testing delay slot filling optimizations...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        result += test_delay_slot_fill(test_val + i);
        result += test_function_call_candidate(test_val + i);
        result += test_asm_candidate(test_val + i);
        result += test_memory_candidate(test_val + i);
        result += test_multiple_jumps(test_val + i);
    }
    
    printf("Final result: %d\n", result);
    printf("(This value is arbitrary - the important part is compilation)\n");
    
    return 0;
}
