/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *p) {
    if (p) *p = (*p * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a < 100);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex test expression */
    if (a > b && glob_c != 0) {
        /* Modify variable used in condition */
        a = a + glob_d;
        /* Additional non-debug instructions */
        glob_c = glob_c ^ 0x12345678;
        sink = a * b;
        /* Another modification */
        a = get_value(a);
        sink += a;
    }
    /* Use result to prevent elimination */
    volatile int local_sink = a + b;
    (void)local_sink;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != glob_b)) {
        /* Modify multiple test expression variables */
        a = a * 2 + 1;
        b = b - glob_c;
        /* Additional arithmetic */
        c = (c << 3) | (d >> 2);
        /* Function call with side effect */
        modify(&glob_d);
        /* More modifications */
        d = get_value(d);
        sink = a + b + c + d;
    }
    /* Ensure side effects are visible */
    volatile int tmp = glob_d;
    (void)tmp;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced value */
    if (ptr && *ptr > threshold && glob_a < 100) {
        /* Modify through pointer - affects test expression */
        *ptr = *ptr * 2 + 5;
        /* Additional operations */
        int temp = *ptr ^ 0xDEADBEEF;
        modify(ptr);
        sink = temp + glob_b;
        /* Another indirect modification */
        *ptr = get_value(*ptr);
    }
    /* Use volatile to prevent optimization */
    volatile int check = (ptr != 0);
    (void)check;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int c = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-modified variables */
        if (a > b || c < glob_c) {
            /* Modify test expression variables */
            a = a + i;
            b = b - get_value(c);
            /* Complex computation */
            c = (c * 3 + a) & 0xFF;
            /* Volatile access to prevent elimination */
            volatile int vol = glob_d;
            c ^= vol;
            sink += a + b + c;
        }
        /* Loop-carried dependency */
        glob_d = (glob_d + 1) & 0xFFFF;
    }
    
    /* Store results to prevent dead code elimination */
    volatile int final_a = a;
    volatile int final_b = b;
    (void)final_a;
    (void)final_b;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_function_condition(int x, int y) {
    /* Condition uses function result */
    if (check_cond(x, y) && (x + y) > glob_c) {
        /* Modify variables used in condition */
        x = x ^ y;
        y = get_value(y);
        /* Multiple assignments */
        glob_c = glob_c * 2;
        x = x + glob_c;
        y = y - 1;
        /* Additional operation */
        sink = x * y;
        /* Call modify function */
        modify(&x);
    }
    
    /* Ensure side effects */
    volatile int result = x + y;
    (void)result;
}

/* Test 6: Mixed operations in then block */
static void __attribute__((noinline, noipa)) test_mixed_operations(int a, int b) {
    /* Simple but non-constant condition */
    if (a != b && glob_a > 0) {
        /* Sequence of assignments modifying test variable */
        a = a + 5;
        a = a * 3;
        a = a ^ 0xAAAAAAAA;
        /* Additional non-modifying operations */
        int temp = b << 2;
        sink = temp;
        /* Another modification */
        a = get_value(a);
        /* Bitwise operation */
        b = b | 0x55555555;
        sink += a + b;
    }
    
    /* Prevent optimization */
    volatile int check = a - b;
    (void)check;
}

int main(void) {
    int test_var = 50;
    int *test_ptr = &test_var;
    
    /* Initialize with non-constant values */
    glob_a = get_value(1);
    glob_b = get_value(2);
    glob_c = get_value(3);
    glob_d = get_value(4);
    
    /* Execute all test cases */
    test_single_modification(glob_a, glob_b);
    test_multiple_modifications(15, 25, 35, 45);
    test_indirect_modification(test_ptr, 30);
    test_loop_nested(10);
    test_function_condition(100, 200);
    test_mixed_operations(300, 400);
    
    /* Additional complex case with volatile */
    volatile int v1 = 123;
    volatile int v2 = 456;
    if (v1 > v2 || glob_c < glob_d) {
        v1 = v1 + glob_a;
        glob_b = glob_b * 2;
        v2 = get_value(v2);
        sink = v1 + v2 + glob_b;
    }
    
    /* Print checksum to verify execution */
    printf("Checksum: %d\n", sink + glob_a + glob_b + glob_c + glob_d + test_var);
    
    return 0;
}
