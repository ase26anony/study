/* Test program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to maintain control over instruction placement */
__attribute__((noinline, optimize("O0")))
static int simple_arithmetic(int a, int b) {
    return a + b;
}

/* Function with compiler barrier to prevent sequence formation */
__attribute__((noinline, optimize("O1")))
static void test_delay_slot_fill_1(void) {
    volatile int x = 0;
    volatile int y = 0;
    int result = 0;
    
    /* Use goto to create a simple jump instruction */
    if (x == 0) {
        goto target_label_1;
    }
    
    /* Dead code to separate the jump from target */
    result = x * y;
    
target_label_1:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Simple arithmetic that doesn't trap
       - Uses local variables (stack memory)
       - No resource conflicts with jump */
    y = x + 1;
    
    /* Use result to prevent elimination */
    result = y;
    printf("Test 1 result: %d\n", result);
}

/* Test with asm statement as candidate */
__attribute__((noinline, optimize("O1")))
static void test_delay_slot_fill_2(void) {
    register int r1 asm("eax") = 10;
    register int r2 asm("ebx") = 20;
    volatile int trigger = 0;
    
    if (trigger == 0) {
        goto target_label_2;
    }
    
    /* Some intermediate code */
    r1 = r1 * 2;
    
target_label_2:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* asm statement candidate:
       - Only modifies general purpose register (eax)
       - No memory access
       - No condition code clobber (avoid "cc")
       - Simple operation that doesn't trap */
    asm volatile("addl $5, %0" : "+r"(r1) :: /* no clobbers */);
    
    /* Use the result */
    printf("Test 2 result: %d\n", r1 + r2);
}

/* Test with function call as candidate */
__attribute__((noinline, optimize("O2")))
static void test_delay_slot_fill_3(void) {
    int a = 5, b = 10;
    volatile int flag = 1;
    
    /* Create simple jump */
    if (flag) {
        goto compute;
    }
    
    a = b = 0;  /* Unreachable but prevents optimization */
    
compute:
    /* Barrier to prevent sequence */
    asm volatile("" ::: "memory");
    
    /* Function call candidate - must not be inlinable */
    int sum = simple_arithmetic(a, b);
    
    printf("Test 3 result: %d\n", sum);
}

/* Test with memory operation that shouldn't trap */
__attribute__((noinline, optimize("O1")))
static void test_delay_slot_fill_4(void) {
    int array[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    int result;
    
    /* Simple jump */
    if (idx >= 0) {
        goto process;
    }
    
    idx = -1;
    
process:
    asm volatile("" ::: "memory");
    
    /* Memory access to stack - should not fault */
    result = array[idx] * 2;
    
    printf("Test 4 result: %d\n", result);
}

/* Complex test with multiple basic blocks */
__attribute__((noinline, optimize("O2")))
static void test_delay_slot_fill_5(void) {
    volatile int counter = 0;
    int total = 0;
    
    /* Loop to create multiple jump opportunities */
    for (int i = 0; i < 3; i++) {
        if (counter == i) {
            goto process_iteration;
        }
        
        /* Alternate path */
        total += i * 10;
        continue;
        
    process_iteration:
        /* Barrier for each label */
        asm volatile("" ::: "memory");
        
        /* Simple non-trapping operation */
        total += i;
        
        /* Another operation to create scheduling opportunity */
        asm volatile("" ::: "memory");
        counter = i + 1;
    }
    
    printf("Test 5 result: %d\n", total);
}

/* Main orchestrator */
int main(void) {
    printf("=== Testing delay slot filling in reorg pass ===\n");
    
    test_delay_slot_fill_1();
    test_delay_slot_fill_2();
    test_delay_slot_fill_3();
    test_delay_slot_fill_4();
    test_delay_slot_fill_5();
    
    printf("=== Tests completed ===\n");
    return 0;
}
