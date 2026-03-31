/* Test program for ifcvt.cc uncovered lines 577-583 */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent constant folding */
    if (a > b && glob_c != 0) {
        /* Modify variable used in condition */
        a = a + glob_d;
        /* Additional non-debug instructions */
        b = b ^ 0x1234;
        sink = a * b;
        /* Call to prevent optimization */
        modify(&glob_c);
    }
    sink = a + b;
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional with multiple variables */
    if ((a > b) || (c < d) || (glob_a != glob_b)) {
        /* Modify multiple variables from condition */
        a = a * 2 + 1;
        b = b | 0xFF00;
        c = c ^ d;
        /* Additional arithmetic */
        d = (d << 3) | (d >> 29);
        /* Use volatile sink */
        sink = a + b + c + d;
        /* Function call with side effect */
        glob_c = get_value(glob_c);
    }
    /* Ensure values are used */
    sink = a ^ b ^ c ^ d;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition uses pointer value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification of what ptr points to */
        *ptr = *ptr + glob_a;
        /* Additional operations */
        int temp = *ptr * 3;
        sink = temp;
        /* Modify global used in condition */
        glob_a = glob_a ^ 0xABCD;
    }
    if (ptr) sink = *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y && (i % 3) != 0) {
            /* Modify test expression variables */
            x = x + i;
            y = y - (glob_c & 1);
            /* Additional statements */
            int z = x * y;
            sink = z;
            /* Function call */
            modify(&glob_d);
        } else {
            /* Alternate path to preserve control flow */
            x = x ^ i;
            y = y | 0x0F;
        }
        /* Loop-carried dependency */
        glob_a = (glob_a + x) & 0xFF;
    }
    sink = x + y;
}

/* Test case 5: Complex arithmetic in condition and modification */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Complex condition with arithmetic */
    if ((a * b > c) && (cond_check(a, b) != 0) && (glob_c < 1000)) {
        /* Modify all variables used in condition */
        a = (a << 2) | (a >> 30);
        b = b * 3 + c;
        c = c ^ get_value(b);
        /* Multiple assignments to flesh out basic block */
        glob_d = glob_d + a;
        sink = b * c;
        /* Additional call */
        modify(&a);
    }
    /* Use results */
    sink = a + b + c;
}

/* Test case 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_access(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Condition using volatile variables */
    if (local_a > local_b || glob_c == 0) {
        /* Modify volatile variables */
        local_a = local_a + 5;
        local_b = local_b * 2;
        /* Access globals */
        glob_c = glob_c ^ local_a;
        /* Multiple operations */
        int temp = local_a * local_b;
        sink = temp;
        /* Function call */
        get_value(local_b);
    }
    sink = local_a - local_b;
}

/* Test case 7: Mixed operations with function calls */
static void __attribute__((noinline, noipa)) test_mixed_operations(int a, int b) {
    /* Condition with function call */
    if (cond_check(a, b) && (a % 7) > (b % 3)) {
        /* Modify test variables */
        a = get_value(a);
        b = b + glob_d;
        /* Additional arithmetic */
        int c = a * b;
        int d = a ^ b;
        sink = c + d;
        /* Nested condition to create more complex CFG */
        if (c > d) {
            a = a << 1;
            modify(&b);
        }
    }
    sink = a * b;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Array for pointer test */
    int data[4] = {50, 60, 70, 80};
    
    printf("Starting ifcvt test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(&data[0], 40);
    result += sink;
    
    test_loop_nested(10);
    result += sink;
    
    test_complex_condition(arg1, arg2, arg3);
    result += sink;
    
    test_volatile_access();
    result += sink;
    
    test_mixed_operations(arg4, arg1);
    result += sink;
    
    /* Final checksum */
    printf("Result checksum: %d\n", result);
    printf("Global state: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
