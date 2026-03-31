/* Test program for GCC reorg pass delay slot filling optimization */
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to create clear function call boundaries */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    /* Simple operation that won't trap */
    return x + 1;
}

__attribute__((noinline, optimize("O0")))
static void memory_barrier(void) {
    /* Prevent instruction merging */
    asm volatile("" ::: "memory");
}

/* Function with simple goto pattern - target architecture: MIPS */
__attribute__((optimize("O2")))
static int test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int result = 0;
    
    /* Use volatile to prevent optimization */
    if (a < b) {
        /* This should generate a simple jump */
        goto target_label1;
    }
    
    /* Unreachable code to create jump opportunity */
    result = 100;
    
target_label1:
    /* Memory barrier to prevent sequence formation */
    memory_barrier();
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Uses local variables
       - No resource conflicts with jump */
    c = a + b;
    
    /* Use result to prevent dead code elimination */
    result = c;
    return result;
}

/* Function with loop and goto - creates multiple jump opportunities */
__attribute__((optimize("O2")))
static int test_pattern2(void) {
    volatile int i, sum = 0;
    volatile int array[4] = {1, 2, 3, 4};
    
    for (i = 0; i < 4; i++) {
        if (array[i] > 2) {
            /* Simple jump to label */
            goto process_label;
        }
        continue;
        
    process_label:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Good delay slot candidate: register-only operation */
        asm volatile("addl $1, %0" : "+r"(sum) :: "cc");
        
        /* Use asm with specific register to avoid resource conflicts */
        int temp = array[i];
        asm volatile("" : "+r"(temp));
        sum += temp;
    }
    
    return sum;
}

/* Function with nested goto patterns */
__attribute__((optimize("O1"), noinline))
static int test_pattern3(int x) {
    volatile int y = x;
    volatile int z = 0;
    
    if (y > 0) {
        goto compute;
    } else {
        goto compute;
    }
    
    /* This code should be unreachable but provides jump target */
    y = y * 2;
    
compute:
    /* Barrier to prevent SEQUENCE formation */
    asm volatile("" ::: "memory");
    
    /* Simple operation that doesn't trap and doesn't use condition codes */
    z = y & 0xFF;
    
    /* Function call as potential delay slot candidate */
    z = safe_operation(z);
    
    return z;
}

/* Function with switch-like goto pattern */
__attribute__((optimize("O2")))
static int test_pattern4(int selector) {
    volatile int a = 0, b = 0;
    
    /* Multiple goto targets to create jump opportunities */
    if (selector == 1) {
        goto case1;
    } else if (selector == 2) {
        goto case2;
    } else {
        goto default_case;
    }
    
case1:
    asm volatile("" ::: "memory");
    /* Simple load/store operation - should not fault */
    a = 1;
    b = a * 10;
    goto end;
    
case2:
    asm volatile("" ::: "memory");
    /* Bitwise operation - won't trap */
    a = 2;
    b = a << 2;
    goto end;
    
default_case:
    asm volatile("" ::: "memory");
    /* Arithmetic operation */
    a = selector;
    b = a + 100;
    
end:
    return b;
}

/* Main orchestrator function */
int main(void) {
    int results[4];
    
    printf("Testing reorg delay slot filling patterns...\n");
    
    /* Execute all test patterns */
    results[0] = test_pattern1();
    results[1] = test_pattern2();
    results[2] = test_pattern3(42);
    results[3] = test_pattern4(2);
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    printf("Total result: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
