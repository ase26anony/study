/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_func(int x) {
    return x + 1;
}

/* Use optimize attribute to control optimization level within functions */
__attribute__((optimize("O2")))
static void test_case_1(void) {
    volatile int a = 10, b = 20, c = 0;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    c = a + b;
    
target_label_1:
    /* Candidate instruction for delay slot filling */
    /* Simple arithmetic that doesn't trap and doesn't conflict with jump */
    asm volatile ("" : "+r" (a) : : "memory");  /* Memory barrier */
    b = a + 1;  /* Simple arithmetic - good candidate */
    
    /* Use the result to prevent dead code elimination */
    printf("Test 1: b = %d\n", b);
}

__attribute__((optimize("O2")))
static void test_case_2(void) {
    volatile int x = 5, y = 0;
    volatile int *ptr = &x;  /* Safe stack pointer */
    
    /* Create multiple basic blocks to encourage reorg optimization */
    if (x > 0) {
        goto compute_label;
    }
    
    /* Dead code to create separation */
    y = x * 2;
    
compute_label:
    /* Memory access to stack variable - should not fault */
    asm volatile ("" ::: "memory");  /* Compiler barrier */
    y = *ptr + 3;  /* Load + arithmetic - potential candidate */
    
    /* Function call as candidate - won't be inlined due to noinline */
    asm volatile ("" ::: "memory");
    y = simple_func(y);
    
    printf("Test 2: y = %d\n", y);
}

__attribute__((optimize("O1")))  /* Lower optimization to preserve jumps */
static void test_case_3(void) {
    register int r1 asm ("t0") = 100;  /* Use register variable */
    register int r2 asm ("t1") = 0;
    
    /* Multiple jumps to same label */
    if (r1 > 50) {
        goto process;
    }
    
    r2 = r1 - 50;
    
process:
    /* Pure register operation - ideal delay slot candidate */
    /* asm ensures specific instruction generation */
    asm volatile ("addiu %0, %1, 5" : "=r" (r2) : "r" (r1) : );
    
    /* Use result */
    printf("Test 3: r2 = %d\n", r2);
}

/* Test with loop structure */
__attribute__((optimize("O2")))
static void test_case_4(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Loop with goto to create jump to label */
    while (counter < 3) {
        if (counter == 1) {
            goto update;
        }
        
        sum += counter;
        counter++;
        continue;
        
    update:
        /* Candidate instruction after label */
        asm volatile ("" ::: "memory");
        sum += 10;  /* Simple assignment */
        counter++;
    }
    
    printf("Test 4: sum = %d\n", sum);
}

/* Test avoiding sequence formation */
__attribute__((optimize("O0")))  /* Minimal optimization to prevent sequences */
static void test_case_5(void) {
    volatile int a = 1, b = 2, c = 3;
    
    /* Simple conditional jump */
    if (a < b) {
        goto do_math;
    }
    
    c = a - b;
    
do_math:
    /* Multiple simple instructions - compiler won't combine into SEQUENCE at O0 */
    asm volatile ("" ::: "memory");
    a = b + c;      /* First simple instruction */
    asm volatile ("" ::: "memory");
    b = a * 2;      /* Second simple instruction */
    
    printf("Test 5: a=%d, b=%d\n", a, b);
}

/* Main orchestrator */
int main(void) {
    printf("Starting delay slot filling tests...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    printf("Tests completed.\n");
    return 0;
}
