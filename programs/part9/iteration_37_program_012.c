/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent function inlining to preserve instruction boundaries */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    /* Simple arithmetic that won't trap */
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline))
static int safe_multiply(int a, int b) {
    /* Multiplication is safe (no division by zero risk) */
    return a * b;
}

/* Function with goto pattern that should generate simple jump */
__attribute__((optimize("O2")))
static int test_goto_pattern_1(void) {
    volatile int result = 0;
    volatile int a = 5, b = 3, c = 7;
    
    /* Use goto to create simple jump instruction */
    if (a > 0) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b + c;
    
target_label_1:
    /* Candidate for delay slot filling - simple arithmetic */
    /* Compiler barrier prevents merging with label */
    asm volatile("" ::: "memory");
    
    /* Simple operation that doesn't trap and has no resource conflicts */
    b = b * 2;  /* Multiplication is safe */
    
    /* Use result to prevent dead code elimination */
    result = b + c;
    return result;
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O2")))
static int test_goto_pattern_2(void) {
    volatile int x = 10;
    volatile int y = 20;
    
    /* Create simple jump */
    if (x > 0) {
        goto compute_target;
    }
    
    /* Unreachable code to create jump structure */
    y = x * 100;
    
compute_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    /* This should be a non-inlineable, non-trapping operation */
    y = simple_operation(x);
    
    return y;
}

/* Test with asm statement as delay slot candidate */
__attribute__((optimize("O2")))
static int test_goto_pattern_3(void) {
    register int r1 asm("eax") = 15;
    register int r2 asm("ebx") = 25;
    
    /* Simple conditional to create jump */
    if (r1 > 10) {
        goto asm_target;
    }
    
    r2 = r1 * 3;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that modifies only specific registers */
    /* Using register constraints to avoid resource conflicts */
    asm volatile(
        "addl $5, %0\n\t"
        : "+r"(r2)  /* Only modifies r2 */
        : 
        : /* No clobbers - avoids cc which might conflict */
    );
    
    return r2;
}

/* Complex pattern with multiple basic blocks */
__attribute__((optimize("O2")))
static int test_complex_pattern(void) {
    volatile int counter = 0;
    volatile int values[4] = {1, 2, 3, 4};
    volatile int sum = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Create goto inside loop */
        if (values[i] > 2) {
            goto process_large;
        }
        
        sum += values[i];
        continue;
        
    process_large:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Safe operation - multiplication doesn't trap */
        sum += values[i] * 2;
        
        /* Another operation to create scheduling opportunity */
        counter++;
    }
    
    return sum + counter;
}

/* Test with nested gotos */
__attribute__((optimize("O1")))  /* O1 for simpler RTL generation */
static int test_nested_goto(void) {
    volatile int a = 1, b = 2, c = 3;
    volatile int result = 0;
    
    if (a) {
        if (b) {
            goto outer_label;
        }
        c = 10;
    }
    
    result = a + b;
    return result;
    
outer_label:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Simple, safe operation */
    result = safe_multiply(b, c);
    
    /* Use goto to return to avoid fall-through issues */
    goto return_point;
    
return_point:
    return result;
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test pattern */
    results[0] = test_goto_pattern_1();
    results[1] = test_goto_pattern_2();
    results[2] = test_goto_pattern_3();
    results[3] = test_complex_pattern();
    results[4] = test_nested_goto();
    
    /* Print results to prevent optimization */
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
