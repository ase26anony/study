/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_computation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int x = 0;
    volatile int y = 0;
    volatile int z = 0;
    
    /* Force compiler to generate a simple jump */
    if (x == 0) {
        goto target_label_1;
    }
    
    /* Some code that won't be reached but prevents optimization */
    y = 1;
    
target_label_1:
    /* Candidate instruction for delay slot filling */
    /* Must be non-jump, non-sequence, and not trap */
    /* Using inline asm with register constraints to control resource usage */
    register int r1 asm("t0") = x;
    asm volatile (
        "addiu %0, %0, 1"
        : "+r" (r1)
        :
        : /* No clobbers - avoids resource conflicts */
    );
    x = r1;
    
    /* Use the result to prevent dead code elimination */
    printf("Result 1: %d\n", x);
}

__attribute__((optimize("O0")))
static void test_case_2(void) {
    volatile int a = 5;
    volatile int b = 10;
    volatile int result = 0;
    
    /* Create multiple basic blocks to encourage jump generation */
    switch (a) {
        case 1: result = 1; break;
        case 2: result = 2; break;
        case 3: result = 3; break;
        case 4: result = 4; break;
        case 5: 
            /* This will generate a jump to the default case */
            goto compute_label;
        default:
compute_label:
            /* Function call as delay slot candidate */
            /* Must not conflict with jump resources */
            result = safe_computation(a, b);
            
            /* Compiler barrier to prevent merging */
            asm volatile("" ::: "memory");
            
            /* Simple arithmetic that won't trap */
            result += 1;
            break;
    }
    
    printf("Result 2: %d\n", result);
}

/* Test with memory operations that shouldn't fault */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    int stack_var1 = 100;
    int stack_var2 = 200;
    int *safe_ptr1 = &stack_var1;
    int *safe_ptr2 = &stack_var2;
    
    /* Force a simple jump */
    if (stack_var1 > 0) {
        goto process_label;
    }
    
    /* Unreachable code to shape control flow */
    safe_ptr1 = NULL;
    
process_label:
    /* Memory operation on stack variable - shouldn't fault */
    *safe_ptr1 = *safe_ptr1 + *safe_ptr2;
    
    /* Another candidate: register operation */
    asm volatile (
        "addu $t0, %1, %2\n\t"
        "move %0, $t0"
        : "=r" (stack_var1)
        : "r" (stack_var1), "r" (stack_var2)
        : "t0"
    );
    
    printf("Result 3: %d\n", stack_var1);
}

/* Test with loop structure that generates jumps */
__attribute__((optimize("O1")))  /* O1 for some optimization but not too aggressive */
static void test_case_4(void) {
    volatile int counter = 0;
    volatile int accumulator = 0;
    
    /* Loop with early exit - generates conditional and unconditional jumps */
    while (counter < 10) {
        if (counter == 5) {
            goto special_case;
        }
        accumulator += counter;
        counter++;
        continue;
        
special_case:
        /* Candidate instruction after label */
        /* Simple operation that doesn't trap */
        accumulator *= 2;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        counter++;
    }
    
    printf("Result 4: %d\n", accumulator);
}

/* Test mixing goto and function calls */
__attribute__((noinline, optimize("O0")))
static int helper_func(int x) {
    return x * 2;
}

static void test_case_5(void) {
    volatile int val = 42;
    volatile int modified = 0;
    
    /* Multiple labels and jumps */
    if (val > 40) {
        goto modify_val;
    }
    
    val = helper_func(val);
    goto end;
    
modify_val:
    /* Good candidate: function call that won't be inlined */
    modified = simple_operation(val);
    
    /* Followed by safe arithmetic */
    asm volatile (
        "sll %0, %0, 1"
        : "+r" (modified)
        :
        : /* No clobbers */
    );
    
end:
    printf("Result 5: %d -> %d\n", val, modified);
}

int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    return 0;
}
