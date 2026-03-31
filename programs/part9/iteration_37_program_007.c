/* test_delay_slot.c
 * 
 * This program is designed to trigger the delay slot filling logic in GCC's reorg pass,
 * specifically targeting lines 2135-2149 of reorg.cc.
 *
 * Compile with: gcc -O2 -march=mips32 -fdump-rtl-reorg -S test_delay_slot.c
 * Or for x86:   gcc -O3 -m32 -fno-gcse -fno-crossjumping -fdump-rtl-reorg -S test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with O0 to prevent premature sequence formation */
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
    /* Candidate for delay slot filling:
     * - Simple arithmetic operation
     * - No memory references that could fault
     * - Doesn't set resources used by the jump
     */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    result = a + b;  /* Simple operation - should not trap */
    asm volatile("" ::: "memory");  /* Prevent merging */
    
    /* Use result to prevent dead code elimination */
    printf("Result 1: %d\n", result);
}

/* Another test case with function call as candidate */
__attribute__((optimize("O0")))
static void test_case_2(void) {
    volatile int x = 5;
    volatile int y;
    
    /* Create simple jump */
    if (x > 0) {
        goto compute;
    }
    
    y = 0;
    return;
    
compute:
    asm volatile("" ::: "memory");
    /* Function call as delay slot candidate */
    y = simple_operation(x);
    asm volatile("" ::: "memory");
    
    printf("Result 2: %d\n", y);
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    register int r1 asm("t0") = 100;
    register int r2 asm("t1") = 200;
    
    /* Simple conditional to create jump */
    if (r1 != 0) {
        goto asm_target;
    }
    
    r2 = 0;
    return;
    
asm_target:
    asm volatile("" ::: "memory");
    /* Simple asm that only modifies a register, no memory, no cc */
    /* Use input/output operands to avoid volatile side effects */
    asm volatile("add %0, %1, %2" 
                 : "=r"(r2) 
                 : "r"(r1), "r"(r2));
    asm volatile("" ::: "memory");
    
    printf("Result 3: %d\n", r2);
}

/* Test with loop structure that might create multiple jump opportunities */
__attribute__((optimize("O1")))  /* O1 for some optimization but not too aggressive */
static void test_case_4(void) {
    volatile int array[4] = {1, 2, 3, 4};
    volatile int sum = 0;
    int i;
    
    for (i = 0; i < 4; i++) {
        /* This if creates a simple jump to skip the increment */
        if (array[i] > 2) {
            goto skip_increment;
        }
        
        sum += array[i];
        continue;
        
    skip_increment:
        asm volatile("" ::: "memory");
        /* Candidate instruction: simple increment */
        sum += 1;
        asm volatile("" ::: "memory");
    }
    
    printf("Result 4: %d\n", sum);
}

/* Test with nested jumps */
__attribute__((optimize("O0")))
static void test_case_5(void) {
    volatile int a = 1, b = 2, c = 3;
    volatile int r;
    
    if (a) {
        if (b) {
            goto inner_label;
        }
        r = a + b;
        return;
    }
    
    r = 0;
    return;
    
inner_label:
    asm volatile("" ::: "memory");
    /* Simple operation that doesn't trap */
    r = c * 2;
    asm volatile("" ::: "memory");
    
    /* Create another jump to same label to increase opportunities */
    if (r > 0) {
        goto inner_label2;
    }
    
    printf("Result 5a: %d\n", r);
    return;
    
inner_label2:
    asm volatile("" ::: "memory");
    r += 5;
    asm volatile("" ::: "memory");
    
    printf("Result 5b: %d\n", r);
}

/* Main function to run all test cases */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    printf("All tests completed.\n");
    return 0;
}
