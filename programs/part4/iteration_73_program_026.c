/* Test program for if-conversion uncovered lines in ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in the test expression */
        a = a + glob_d;
        /* Additional non-debug instructions */
        glob_c = glob_c ^ a;
        sink = a * b;
        /* Another modification to ensure multiple instructions */
        a = get_value(a);
    }
    sink += a + b;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != glob_b)) {
        /* Modify 'a' and 'c' which appear in test expression */
        a = a * 2 + 1;
        /* Intermediate computation */
        int temp = b ^ c;
        /* Modify 'c' */
        c = c - d + temp;
        /* Additional arithmetic */
        glob_d = glob_d ^ (a | c);
        /* Another modification to 'a' */
        a = (a << 3) | (a >> 29);  /* Rotate left by 3 */
        sink += a + c;
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through pointer - affects *ptr used in condition */
        *ptr = *ptr / 2;
        /* Additional operations */
        modify(ptr);
        int temp = *ptr ^ 0x12345678;
        *ptr = temp & 0xFFFF;
        sink += *ptr;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (a > b || cond_check(a, b)) {
            /* Modify variables used in condition */
            a = a + i;
            b = b - (i & 1);
            /* Additional computations */
            int c = a * b;
            a = (a ^ c) & 0xFF;
            sink += a + b + c;
        }
        /* Loop-carried dependency */
        glob_c = (glob_c + a) & 0xFF;
    }
    sink += a + b;
}

/* Test 5: Complex test with function calls in condition */
static void __attribute__((noinline, noipa)) test_function_in_condition(int x, int y) {
    /* Condition with function call */
    if (get_value(x) > y && cond_check(x, y)) {
        /* Modify 'x' which is used in function arguments for condition */
        x = x * 3 + 7;
        /* Multiple instructions in then block */
        y = y ^ x;
        modify(&x);
        /* Volatile access to prevent optimization */
        sink = x + y + glob_d;
        /* Another modification */
        x = (x << 2) | (x >> 30);
    }
    sink += x + y;
}

/* Test 6: Mixed operations with volatile in test expression */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    volatile int vol = sink;
    
    /* Test expression uses volatile and non-volatile */
    if (local_a > vol && local_b < glob_c) {
        /* Modify variables used in condition */
        local_a = local_a + vol;
        local_b = local_b * 2;
        /* Multiple arithmetic operations */
        int t1 = local_a ^ local_b;
        int t2 = t1 * 3;
        local_a = t2 & 0xFF;
        sink = local_a + local_b + t1;
    }
    sink += local_a + local_b;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Allocate memory for pointer test */
    int *ptr = malloc(sizeof(int));
    if (ptr) {
        *ptr = 1000;
    }
    
    /* Execute test cases */
    test_single_modification(arg1, arg2);
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    
    if (ptr) {
        test_indirect_modification(ptr, 500);
    }
    
    test_loop_nested(5);
    test_function_in_condition(arg1, arg2);
    test_volatile_mix();
    
    /* Cleanup and final result */
    if (ptr) {
        result = *ptr;
        free(ptr);
    }
    
    result += sink;
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result);
    printf("globals: a=%d b=%d c=%d d=%d\n", glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
