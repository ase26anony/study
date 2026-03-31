/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) static int simple_func(int x) {
    return x + 1;
}

/* Another noinline function for delay slot candidate */
__attribute__((noinline)) static int another_func(int x) {
    return x * 2;
}

/* Function with O0 optimization to prevent instruction merging */
__attribute__((optimize("O0"))) 
static void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    
    /* Use goto to create simple jump */
    if (a > 5) {
        goto target_label1;
    }
    
    /* Some code to prevent fall-through optimization */
    b = 30;
    
target_label1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    c = a + b;
    
    /* Use the result to prevent dead code elimination */
    printf("Pattern1: %d\n", c);
}

/* Function with memory operations */
__attribute__((optimize("O0")))
static void test_pattern2(void) {
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int i = 0, sum = 0;
    
    /* Create simple jump */
    if (array[0] > 0) {
        goto compute;
    }
    
    /* Dummy code */
    i = 100;
    
compute:
    /* Safe memory access (stack variable, won't fault) */
    sum = array[5] + array[6];
    
    /* Use result */
    printf("Pattern2: %d\n", sum);
}

/* Function using asm statements to control resource usage */
__attribute__((optimize("O0")))
static void test_pattern3(void) {
    int x = 5, y = 10, result = 0;
    
    /* Simple jump */
    if (x < 10) {
        goto asm_target;
    }
    
    y = 20;
    
asm_target:
    /* asm statement that only modifies a specific register
       and doesn't reference memory or condition codes */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(result)      /* output operand in register */
        : "r"(x)            /* input operand in register */
        /* no clobbers - avoids conflicts with jump's resource set */
    );
    
    printf("Pattern3: %d\n", result);
}

/* Function with function call as delay slot candidate */
__attribute__((optimize("O1")))  /* O1 allows some scheduling but not too aggressive */
static void test_pattern4(void) {
    int val = 42;
    int ret = 0;
    
    /* Compiler barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    /* Simple jump */
    if (val > 0) {
        goto call_func;
    }
    
    val = 100;
    
call_func:
    /* Function call - must not be inlinable and must not throw */
    ret = simple_func(val);
    
    /* Use result */
    printf("Pattern4: %d\n", ret);
}

/* Complex pattern with multiple jumps */
__attribute__((optimize("O2")))
static void test_pattern5(void) {
    volatile int counter = 0;
    int temp1 = 0, temp2 = 0;
    
    /* First simple jump */
    if (counter == 0) {
        goto label_a;
    }
    
    temp1 = 50;
    
label_a:
    /* First candidate - simple arithmetic */
    temp1 = counter + 1;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Second simple jump */
    if (temp1 > 0) {
        goto label_b;
    }
    
    temp2 = 100;
    
label_b:
    /* Second candidate - another simple operation */
    temp2 = temp1 * 2;
    
    printf("Pattern5: %d, %d\n", temp1, temp2);
}

/* Test with loop to create more scheduling opportunities */
static void test_pattern6(void) {
    int i, sum = 0;
    int data[5] = {1, 2, 3, 4, 5};
    
    for (i = 0; i < 5; i++) {
        /* Simple jump inside loop */
        if (data[i] > 2) {
            goto process;
        }
        
        sum += 10;
        continue;
        
    process:
        /* Candidate instruction - safe memory access */
        sum += data[i];
        
        /* Compiler barrier to prevent sequence formation */
        asm volatile("" ::: "memory");
    }
    
    printf("Pattern6: %d\n", sum);
}

/* Main function to run all tests */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    test_pattern6();
    
    return 0;
}
