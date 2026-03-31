/* Test program for GCC reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) int safe_operation(int x) {
    return x + 1;
}

__attribute__((noinline)) int another_safe_op(int x) {
    return x * 2;
}

/* Use volatile to prevent optimization of asm statements */
static volatile int global_counter = 0;

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0"))) 
void test_simple_jump_with_eligible_instruction(void) {
    int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple goto that will generate a simplejump_p instruction */
    if (a < b) {
        goto target_label;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b;
    
target_label:
    /* Compiler barrier to prevent merging with the jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic that doesn't trap
       - No memory references that could fault
       - Doesn't set resources conflicting with the jump
    */
    c = a + b;  /* Simple arithmetic - good candidate */
    
    /* Use the result to prevent dead code elimination */
    global_counter += c;
}

/* Test with function call as candidate */
__attribute__((optimize("O0")))
void test_jump_with_function_call(void) {
    int x = 5, y = 0;
    
    if (x > 0) {
        goto call_target;
    }
    
    y = x * 2;
    
call_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as candidate - must not be inlinable */
    y = safe_operation(x);
    
    global_counter += y;
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
void test_jump_with_asm_instruction(void) {
    int reg1 = 1, reg2 = 2, reg3 = 3;
    
    if (reg1 < reg2) {
        goto asm_target;
    }
    
    reg3 = reg1 - reg2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Simple asm instruction that:
       - Only modifies a general purpose register
       - Doesn't touch condition codes (no "cc" clobber)
       - Doesn't access memory
    */
    asm volatile("add %1, %0" 
                 : "+r" (reg3) 
                 : "r" (reg1)
                 : /* no clobbers */);
    
    global_counter += reg3;
}

/* Test with multiple jumps to same label */
__attribute__((optimize("O0")))
void test_multiple_jumps_same_target(void) {
    int i, total = 0;
    int values[4] = {1, 2, 3, 4};
    
    for (i = 0; i < 4; i++) {
        if (values[i] % 2 == 0) {
            goto process_even;
        } else {
            goto process_odd;
        }
        
        /* These won't be reached but create the jump structure */
        continue;
        
    process_even:
        asm volatile("" ::: "memory");
        total += values[i] * 2;  /* Candidate instruction */
        continue;
        
    process_odd:
        asm volatile("" ::: "memory");
        total += values[i] * 3;  /* Candidate instruction */
        continue;
    }
    
    global_counter += total;
}

/* Test with nested control flow */
__attribute__((optimize("O0")))
void test_nested_jumps(void) {
    int a = 10, b = 20, c = 0;
    int i;
    
    for (i = 0; i < 3; i++) {
        if (a > b) {
            goto outer_label;
        }
        
        if (i == 1) {
            goto inner_label;
        }
        
        c += i;
        continue;
        
    inner_label:
        asm volatile("" ::: "memory");
        /* Simple instruction that doesn't trap */
        c = a - b;
        continue;
    }
    
outer_label:
    asm volatile("" ::: "memory");
    c = b - a;  /* Another candidate */
    
    global_counter += c;
}

/* Test with switch statement generating simple jumps */
__attribute__((optimize("O0")))
void test_switch_with_goto(void) {
    int x = 2;
    int result = 0;
    
    switch (x) {
        case 1:
            goto case1;
        case 2:
            goto case2;
        case 3:
            goto case3;
        default:
            goto default_case;
    }
    
case1:
    asm volatile("" ::: "memory");
    result = x + 100;
    goto end;
    
case2:
    asm volatile("" ::: "memory");
    result = x * 50;  /* Candidate */
    goto end;
    
case3:
    asm volatile("" ::: "memory");
    result = x - 10;
    goto end;
    
default_case:
    asm volatile("" ::: "memory");
    result = x / 2;  /* Careful: division might trap! Use safe value */
    goto end;
    
end:
    global_counter += result;
}

/* Main function to run all tests */
int main(void) {
    printf("Testing delay slot filling optimization...\n");
    
    test_simple_jump_with_eligible_instruction();
    printf("Test 1 completed\n");
    
    test_jump_with_function_call();
    printf("Test 2 completed\n");
    
    test_jump_with_asm_instruction();
    printf("Test 3 completed\n");
    
    test_multiple_jumps_same_target();
    printf("Test 4 completed\n");
    
    test_nested_jumps();
    printf("Test 5 completed\n");
    
    test_switch_with_goto();
    printf("Test 6 completed\n");
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    return 0;
}
