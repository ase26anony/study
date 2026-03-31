/* Test program for GCC reorg.cc delay slot optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

__attribute__((noinline))
static void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Function with a simple jump to label followed by eligible instruction */
__attribute__((optimize("O2")))
static int test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    int result = 0;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b;
    
target_label:
    /* Candidate instruction for delay slot:
       - Simple arithmetic operation
       - No memory access that could fault
       - Doesn't set resources used by the jump
       - Not a jump itself
    */
    memory_barrier();  /* Prevent merging with label */
    
    /* Simple arithmetic that won't trap */
    result = b + 1;
    
    /* Use the result to prevent dead code elimination */
    return result;
}

/* Test with inline asm that doesn't conflict with jump resources */
__attribute__((optimize("O2")))
static int test_case_2(void) {
    int x = 5;
    int y = 10;
    
    /* Force a simple jump */
    if (x < 10) {
        goto asm_target;
    }
    
    y = x * 2;
    
asm_target:
    memory_barrier();
    
    /* Inline asm that:
       - Only modifies a general purpose register (eax)
       - Doesn't touch condition codes (no "cc" clobber)
       - Doesn't access memory
    */
    asm volatile (
        "addl $1, %0"
        : "+r"(x)  /* Only modifies x through register */
        :          /* No inputs */
        :          /* No clobbers - important! */
    );
    
    return x + y;
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O2")))
static int test_case_3(void) {
    volatile int counter = 0;
    int value = 5;
    
    /* Multiple basic blocks to encourage reorg optimization */
    for (int i = 0; i < 3; i++) {
        if (counter > 100) {
            goto call_target;
        }
        counter++;
    }
    
    value = 0;
    
call_target:
    memory_barrier();
    
    /* Function call that doesn't throw and has no resource conflicts */
    value = simple_operation(value);
    
    return value;
}

/* Test case specifically designed for the uncovered conditions:
   - jump_to_label_p(trial) == true
   - simplejump_p(trial) == true  
   - next_trial is a non-jump, non-SEQUENCE instruction
   - next_trial doesn't reference/set conflicting resources
   - next_trial doesn't trap
   - eligible_for_delay returns true
*/
__attribute__((optimize("O2")))
static int test_delay_slot_filling(void) {
    int arr[4] = {1, 2, 3, 4};
    int sum = 0;
    int i = 0;
    
    /* Complex enough control flow to avoid early optimization */
    while (i < 4) {
        if (arr[i] > 2) {
            /* This should generate a simple jump to label */
            goto delay_candidate;
        }
        sum += arr[i];
        i++;
    }
    
    /* Alternate path */
    sum *= 2;
    goto end;
    
delay_candidate:
    /* Compiler barrier to prevent instruction merging */
    asm volatile("" ::: "memory");
    
    /* Ideal delay slot candidate:
       - Simple register operation
       - No memory access (avoid potential faults)
       - No function call (avoid internal throws)
       - No condition code modification
    */
    int temp = sum;
    temp = temp + arr[i];  /* Safe array access - index is bounded */
    
    /* Use inline asm for precise control */
    asm volatile (
        "addl %1, %0"
        : "+r"(temp)
        : "r"(i)
        : /* No clobbers! */
    );
    
    sum = temp;
    i++;
    
end:
    return sum;
}

/* Main function to run all test cases */
int main(void) {
    int results[4];
    
    printf("Testing delay slot optimization patterns...\n");
    
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_delay_slot_filling();
    
    printf("Results: %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3]);
    
    return 0;
}
