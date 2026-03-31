/* test_reorg.c - Target specific delay slot filling for reorg.cc lines 2135-2149 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

__attribute__((noinline, optimize("O0")))
static void test_simple_jump_with_arithmetic(void) {
    volatile int a = 0, b = 0, c = 0;
    
    /* Use volatile to prevent optimization */
    volatile int *ptr = &a;
    
    /* Create a simple goto that will become a simplejump_p */
    if (a == 0) {
        goto target_label;
    }
    
    /* Some code to avoid fall-through optimization */
    b = 1;
    c = 2;
    
target_label:
    /* Candidate for next_trial: simple arithmetic that doesn't trap */
    /* Compiler barrier to prevent merging with label */
    asm volatile("" ::: "memory");
    
    /* Simple arithmetic - won't trap, not a jump, not a sequence */
    a = b + c;
    
    /* Use the result to prevent dead code elimination */
    *ptr = a;
}

__attribute__((noinline, optimize("O0")))
static void test_jump_to_function_call(void) {
    int x = 5;
    int result = 0;
    
    /* Simple conditional to create goto */
    if (x > 0) {
        goto call_target;
    }
    
    /* Dead code to separate the jump */
    x = x * 2;
    
call_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as candidate - must not be inlined */
    result = simple_operation(x);
    
    /* Use result */
    printf("Result: %d\n", result);
}

__attribute__((noinline, optimize("O0")))
static void test_asm_instruction_candidate(void) {
    register int r1 asm("eax") = 10;
    register int r2 asm("ebx") = 20;
    
    /* Create simple jump */
    if (r1 < r2) {
        goto asm_target;
    }
    
    /* Some intervening code */
    r1 = r1 * 2;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* ASM instruction that modifies only specific registers */
    /* Doesn't reference memory, doesn't trap */
    asm volatile("addl %1, %0" 
                 : "+r"(r1) 
                 : "r"(r2) 
                 : /* no clobbers - avoid cc to prevent resource conflict */);
    
    /* Use the result */
    printf("ASM result: %d\n", r1);
}

__attribute__((noinline, optimize("O0")))
static void test_memory_operation_no_trap(void) {
    /* Stack variables are guaranteed safe addresses */
    int array[4] = {1, 2, 3, 4};
    volatile int index = 0;
    int temp = 0;
    
    /* Simple jump */
    if (array[0] > 0) {
        goto mem_target;
    }
    
    /* Code to separate */
    index = 1;
    
mem_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory load from safe stack address - shouldn't trap */
    temp = array[index];
    
    /* Use result */
    array[0] = temp;
}

__attribute__((noinline, optimize("O0")))
static void test_multiple_candidates_in_loop(void) {
    int i;
    volatile int counter = 0;
    
    for (i = 0; i < 10; i++) {
        /* Create simple jumps inside loop */
        if (i % 2 == 0) {
            goto loop_target;
        }
        
        /* Some code */
        counter++;
        continue;
        
    loop_target:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Simple arithmetic candidate */
        counter = counter + i;
    }
    
    printf("Loop counter: %d\n", counter);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_simple_jump_with_arithmetic();
    test_jump_to_function_call();
    test_asm_instruction_candidate();
    test_memory_operation_no_trap();
    test_multiple_candidates_in_loop();
    
    return 0;
}
