/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_delay_slot_fill_1(void) {
    volatile int a = 0, b = 0, c = 0;
    int result = 0;
    
    /* Use goto to create a simple jump */
    if (a == 0) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed */
    b = 1;
    
target_label_1:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic operation (won't trap)
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    
    /* Simple arithmetic that doesn't trap */
    c = a + 5;
    
    /* Use the result to prevent elimination */
    result = c;
    
    /* Another barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    printf("Test 1 result: %d\n", result);
}

/* Another test with function call as candidate */
__attribute__((optimize("O0")))
static void test_delay_slot_fill_2(void) {
    volatile int x = 10;
    int result = 0;
    
    /* Create simple jump */
    if (x > 0) {
        goto target_label_2;
    }
    
    /* Unreachable code */
    x = -1;
    
target_label_2:
    asm volatile("" ::: "memory");  /* Barrier */
    
    /* Function call as delay slot candidate */
    result = simple_operation(x);
    
    asm volatile("" ::: "memory");  /* Barrier */
    
    printf("Test 2 result: %d\n", result);
}

/* Test with register-only operations to avoid resource conflicts */
__attribute__((optimize("O0")))
static void test_delay_slot_fill_3(void) {
    register int r1 asm("t0") = 1;  /* Use call-clobbered register on MIPS */
    register int r2 asm("t1") = 2;
    register int r3 asm("t2") = 0;
    
    /* Simple jump */
    if (r1 > 0) {
        goto target_label_3;
    }
    
    r1 = 0;
    
target_label_3:
    asm volatile("" ::: "memory");
    
    /* Register-only operation - won't trap, no memory access */
    /* Use inline asm with specific register constraints */
    asm volatile(
        "addu %0, %1, %2\n\t"
        : "=r" (r3)
        : "r" (r1), "r" (r2)
    );
    
    asm volatile("" ::: "memory");
    
    printf("Test 3 result: %d\n", r3);
}

/* Test with multiple basic blocks to encourage reorg optimization */
__attribute__((optimize("O2")))  /* Higher optimization for this one */
static void test_delay_slot_fill_4(void) {
    volatile int counter = 0;
    int i, sum = 0;
    
    for (i = 0; i < 10; i++) {
        /* Create pattern with simple jumps */
        if (i % 2 == 0) {
            goto even_label;
        }
        
        /* Odd case */
        sum += i * 3;
        continue;
        
    even_label:
        asm volatile("" ::: "memory");
        
        /* Simple operation at jump target */
        sum += i * 2;
        
        asm volatile("" ::: "memory");
    }
    
    printf("Test 4 sum: %d\n", sum);
}

/* Test that avoids all trapping operations */
__attribute__((optimize("O0")))
static void test_delay_slot_fill_5(void) {
    volatile int x = 100;
    volatile int y = 200;
    int z = 0;
    
    /* Nested jumps to create opportunities */
    if (x > 50) {
        if (y > 100) {
            goto target_label_5;
        }
    }
    
    z = -1;
    goto end;
    
target_label_5:
    asm volatile("" ::: "memory");
    
    /* Safe operations only:
       - No division (could trap)
       - No memory access through pointers (could fault)
       - Only local variable operations
    */
    z = x & y;      /* Bitwise AND - never traps */
    z = z | 0x01;   /* Bitwise OR - never traps */
    z = z ^ 0xFF;   /* Bitwise XOR - never traps */
    
    asm volatile("" ::: "memory");
    
end:
    printf("Test 5 result: %d\n", z);
}

int main(void) {
    printf("Testing delay slot filling optimization...\n");
    
    test_delay_slot_fill_1();
    test_delay_slot_fill_2();
    test_delay_slot_fill_3();
    test_delay_slot_fill_4();
    test_delay_slot_fill_5();
    
    return 0;
}
