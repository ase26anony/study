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
static void safe_memory_op(int *ptr) {
    *ptr = *ptr + 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int a = 0, b = 0, c = 0;
    
    /* Create a simple jump to a label */
    if (a == 0) {
        goto target_label_1;
    }
    
    /* Some code to prevent fall-through optimization */
    b = 1;
    c = 2;
    
target_label_1:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic operation
       - No memory access that might fault
       - Doesn't set resources used by the jump
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    a = b + c;  /* Simple arithmetic - should not trap */
    
    /* Use the result to prevent dead code elimination */
    printf("Test 1: %d\n", a);
}

__attribute__((optimize("O0")))
static void test_case_2(void) {
    int x = 0, y = 0;
    volatile int trigger = 1;
    
    /* Force a simple jump */
    if (trigger) {
        goto target_label_2;
    }
    
    /* Dead code that won't execute */
    x = 100;
    
target_label_2:
    /* Another candidate: function call that doesn't throw */
    asm volatile("" ::: "memory");
    y = simple_operation(x);
    
    printf("Test 2: %d\n", y);
}

__attribute__((optimize("O0")))
static void test_case_3(void) {
    int arr[10] = {0};
    int i = 0;
    volatile int cond = 1;
    
    /* Jump to label */
    if (cond) {
        goto compute_label;
    }
    
    /* Unreachable code */
    i = 100;
    
compute_label:
    /* Safe memory operation on stack variable (won't fault) */
    asm volatile("" ::: "memory");
    safe_memory_op(&arr[0]);
    
    printf("Test 3: %d\n", arr[0]);
}

/* Test with inline asm that has specific register usage */
__attribute__((optimize("O0")))
static void test_case_4(void) {
    int r1 = 10, r2 = 20, r3 = 0;
    volatile int jump_flag = 1;
    
    if (jump_flag) {
        goto asm_target;
    }
    
    r1 = 0;  /* Unreachable */
    
asm_target:
    /* Inline asm that only modifies a specific register
       and doesn't reference memory or condition codes */
    asm volatile("" ::: "memory");  /* Barrier */
    asm volatile("add %1, %2, %0" 
                 : "=r"(r3) 
                 : "r"(r1), "r"(r2));
    
    printf("Test 4: %d + %d = %d\n", r1, r2, r3);
}

/* Complex test with multiple jumps */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static void test_case_5(void) {
    int counter = 0;
    int result = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Create pattern of jumps that might trigger delay slot filling */
        if (i % 2 == 0) {
            goto even_case;
        }
        
        result += i * 2;
        continue;
        
    even_case:
        /* Candidate instruction: simple increment */
        asm volatile("" ::: "memory");
        counter++;
        result += i;
    }
    
    printf("Test 5: counter=%d, result=%d\n", counter, result);
}

/* Test with nested jumps */
static void test_case_6(void) {
    volatile int a = 5, b = 10;
    int temp = 0;
    
    if (a < b) {
        goto outer_label;
    }
    
    temp = 100;
    return;
    
outer_label:
    /* First candidate */
    asm volatile("" ::: "memory");
    temp = a + b;
    
    if (temp > 10) {
        goto inner_label;
    }
    
    printf("Should not reach here\n");
    return;
    
inner_label:
    /* Second candidate */
    asm volatile("" ::: "memory");
    a = temp * 2;
    
    printf("Test 6: a=%d, temp=%d\n", a, temp);
}

int main(void) {
    printf("Starting delay slot filling tests...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    test_case_6();
    
    printf("Tests completed.\n");
    return 0;
}
