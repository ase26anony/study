/* Test program for if-conversion uncovered lines in ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink;  /* To prevent dead code elimination */

/* Non-inlineable helper functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Condition uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable used in condition */
        a = a + glob_b;  // This should trigger modified_in_p
        /* Additional non-debug instructions */
        glob_c = glob_c ^ 0x1234;
        sink = a * b;
    }
    sink = a;  /* Use result */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound condition with multiple variables */
    if ((a > b) || (c < d && glob_a != 0)) {
        /* Modify multiple variables from condition */
        a = get_value(a);  // Modifies 'a' from condition
        c = c * 2 + 1;     // Modifies 'c' from condition
        /* Additional arithmetic */
        b = (b << 3) | 0x7;
        sink = a + b + c + d;
    }
    /* Use results to prevent optimization */
    glob_a = a;
    glob_b = b;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition uses pointer and value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification through pointer */
        *ptr = *ptr / 2;  // Modifies what ptr points to
        /* Additional operations */
        int temp = *ptr + threshold;
        modify(&temp);
        sink = temp;
    }
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses loop-varying variables */
        if (a > b && (i % 3) != 0) {
            /* Modify variables used in condition */
            a = a + i;  // This modification should be analyzed
            b = b - (i & 1);
            /* Additional computation */
            int c = a ^ b;
            sink = c;
        }
        /* Loop-carried dependency */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = (b * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    glob_a = a;
    glob_b = b;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Non-trivial condition using function call */
    if (check_cond(x, y) && (x & y) != 0) {
        /* Modify variables from condition */
        x = x | 0xF0F0F0F0;  // Modifies 'x'
        y = y & 0x0F0F0F0F;  // Modifies 'y'
        /* Sequence of assignments */
        int z = x + y;
        z = z * 3;
        z = z ^ 0xAAAAAAAA;
        sink = z;
    }
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Condition uses volatile variables */
    if (local_a > local_b && glob_c > 0) {
        /* Modify volatile variable used in condition */
        local_a = local_a + glob_c;  // Should be detected
        /* Additional volatile operations */
        glob_c = glob_c - 1;
        sink = local_a * local_b;
    }
}

/* Main driver */
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
    
    if (dynamic_ptr) {
        test_indirect_modification(dynamic_ptr, 100);
        result += sink;
    }
    
    test_loop_nested(5);
    result += sink;
    
    test_complex_condition(arg1, arg2);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    /* Cleanup */
    if (dynamic_ptr) {
        free(dynamic_ptr);
    }
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result & 0xFF);
    
    return 0;
}
