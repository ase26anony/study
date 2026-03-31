/* Test program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with optimization disabled to prevent instruction merging */
__attribute__((optimize("O0")))
static void test_simple_jump_with_arithmetic(void) {
    volatile int a = 0, b = 0, c = 0;
    int result = 0;
    
    /* Use volatile to prevent optimization */
    volatile int trigger = 1;
    
    if (trigger) {
        /* Simple goto that should generate a simplejump_p instruction */
        goto target_label;
    }
    
    /* Dead code that won't be reached but prevents optimization */
    a = b + c;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate for delay slot: simple arithmetic that doesn't trap */
    /* This should be NONJUMP_INSN_P and not a SEQUENCE */
    result = a + 5;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
}

/* Another test with function call after label */
__attribute__((optimize("O0")))
static void test_jump_with_function_call(void) {
    volatile int x = 10;
    int y = 0;
    
    volatile int flag = 1;
    
    if (flag) {
        goto call_target;
    }
    
    /* Unreachable code */
    x = x * 2;
    
call_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call that doesn't inline - potential delay slot candidate */
    y = simple_operation(x);
    
    printf("Function result: %d\n", y);
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static void test_jump_with_asm(void) {
    int reg_var = 0;
    volatile int cond = 1;
    
    if (cond) {
        goto asm_target;
    }
    
    reg_var = 100;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Simple asm that modifies a register but doesn't trap */
    /* Using "+r" constraint to ensure it's a register operation */
    asm volatile("addl $1, %0" : "+r"(reg_var) :: "cc");
    
    printf("ASM result: %d\n", reg_var);
}

/* More complex test with multiple basic blocks */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static void test_nested_jumps(void) {
    volatile int i, j = 0;
    int sum = 0;
    
    for (i = 0; i < 10; i++) {
        volatile int inner_flag = (i & 1);
        
        if (inner_flag) {
            goto compute;
        }
        
        j = i * 2;
        continue;
        
    compute:
        /* Candidate instruction: simple arithmetic on local variable */
        /* Should not trap and not conflict with jump resources */
        sum += i;
        
        /* Use asm to prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    printf("Nested sum: %d\n", sum);
}

/* Test that creates multiple jump-to-label patterns */
__attribute__((noinline, optimize("O0")))
static int test_multiple_labels(int input) {
    int output = input;
    volatile int switch_val = input % 4;
    
    /* Multiple goto patterns to increase chances */
    switch (switch_val) {
        case 0:
            goto label0;
        case 1:
            goto label1;
        case 2:
            goto label2;
        default:
            goto label3;
    }
    
label0:
    asm volatile("" ::: "memory");
    output += 1;
    goto end;
    
label1:
    asm volatile("" ::: "memory");
    output += 2;
    goto end;
    
label2:
    asm volatile("" ::: "memory");
    output += 3;
    goto end;
    
label3:
    asm volatile("" ::: "memory");
    output += 4;
    
end:
    return output;
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test patterns */
    test_simple_jump_with_arithmetic();
    test_jump_with_function_call();
    test_jump_with_asm();
    test_nested_jumps();
    
    int result = test_multiple_labels(5);
    printf("Multiple labels result: %d\n", result);
    
    /* Additional test with memory operation that shouldn't trap */
    {
        int safe_array[10] = {0};
        volatile int idx = 5;
        int val = 0;
        
        if (idx < 10) {
            goto safe_load;
        }
        
        val = -1;
        goto done;
        
    safe_load:
        /* Memory access to stack variable - should not trap */
        val = safe_array[idx];
        
    done:
        printf("Safe load result: %d\n", val);
    }
    
    return 0;
}
