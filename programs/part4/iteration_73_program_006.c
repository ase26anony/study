/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent constant folding */
    if (a > b && (get_value(a) != 0)) {
        /* Modify variable used in condition */
        a = a + (b & 0xFF);
        /* Additional non-debug instructions */
        sink = a * 3;
        b = b ^ 0x1234;
        /* Another modification of test expression variable */
        a = a | 0x1;
        sink += b;
    }
    sink += a;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound conditional with multiple variables */
    if ((a > b) || (c < d) || ((a + c) > (b + d))) {
        /* Modify multiple test expression variables */
        a = a * 2 + 1;
        c = c - d;
        /* Additional arithmetic to flesh out basic block */
        b = b ^ a;
        d = d * 3;
        /* Function call that might modify state */
        modify(&a);
        sink = a + b + c + d;
    }
    sink += a + c;
}

/* Test 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses pointer and value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification through pointer */
        *ptr = *ptr + threshold;
        /* Additional operations */
        int temp = *ptr & 0xFF;
        *ptr = *ptr ^ temp;
        sink = *ptr;
        /* Another pointer-based modification */
        modify(ptr);
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (check_cond(a, b) || (a + i > b)) {
            /* Modify test expression variables */
            a = a + (i & 0xF);
            b = b - (i % 7);
            /* Additional computations */
            int c = a * b;
            sink += c;
            /* Another modification */
            a = a ^ 0xABCD;
        }
        /* Loop-carried dependency */
        b = b + 1;
        sink += a;
    }
    glob_a = a;
    glob_b = b;
}

/* Test 5: Complex condition with volatile reads */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    int c = glob_c;
    
    /* Condition with volatile and non-volatile variables */
    if (local_a > local_b && c != 0 && (get_value(local_a) > get_value(local_b))) {
        /* Modify variables used in condition */
        local_a = local_a + c;
        c = c * 2;
        /* Additional statements */
        int d = local_a ^ local_b;
        sink += d;
        /* Function call */
        modify(&c);
        local_b = local_b - 1;
    }
    sink += local_a + c;
}

/* Test 6: Bitwise condition with modification */
static void __attribute__((noinline, noipa)) test_bitwise_ops(int x, int y, int mask) {
    /* Bitwise condition */
    if ((x & mask) && (y | mask) != mask && (x ^ y) > 0) {
        /* Modify test expression variables */
        x = x | (y & 0xFF);
        y = y ^ (x >> 4);
        /* Multiple assignments */
        mask = mask << 1;
        x = x + (mask & 0xF);
        sink = x + y + mask;
        /* Another modification */
        modify(&y);
    }
    sink += x;
}

/* Test 7: Function call in condition with modification */
static int __attribute__((noinline, noipa)) condition_func(int *a, int *b) {
    return (*a > *b) && ((*a ^ *b) & 1);
}

static void __attribute__((noinline, noipa)) test_func_cond(int a, int b) {
    /* Condition via function call */
    if (condition_func(&a, &b)) {
        /* Modify variables passed to condition function */
        a = a * 3 + 7;
        b = b / 2;
        /* Additional operations */
        int c = a & b;
        sink += c;
        a = a ^ 0xDEADBEEF;
        b = b + 0xCAFE;
    }
    sink += a - b;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Dynamic allocation for pointer test */
    int *dynamic_ptr = malloc(sizeof(int));
    if (dynamic_ptr) {
        *dynamic_ptr = 150;
    }
    
    /* Run all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multi_modification(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(dynamic_ptr, 100);
    result += sink;
    
    test_loop_nested(5);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    test_bitwise_ops(arg1, arg2, 0xF0F0F0F0);
    result += sink;
    
    test_func_cond(arg3, arg4);
    result += sink;
    
    /* Cleanup */
    if (dynamic_ptr) {
        free(dynamic_ptr);
    }
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result & 0xFF);
    
    return 0;
}
