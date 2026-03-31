/* Test program for if-conversion coverage: ifcvt.cc lines 577-583 */
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
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr = (*ptr * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Condition uses 'a' */
    if (a > b) {
        /* Modify 'a' which is used in the condition */
        a = a + 1;
        /* Additional non-debug instructions */
        sink = sink ^ a;
        a = a * 2;
        sink = sink + b;
    }
    /* Use result to prevent dead code elimination */
    sink = sink + a;
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound condition using multiple variables */
    if ((a > b) && (c != d)) {
        /* Modify both 'a' and 'c' from the condition */
        a = a ^ 0x12345678;
        sink = sink | a;
        c = c + b;
        sink = sink ^ c;
        /* Additional arithmetic */
        b = b - 1;
        sink = sink + b;
    }
    /* Ensure results are used */
    sink = sink + a + c;
}

/* Test 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition checks pointer and value */
    if (ptr && (*ptr > threshold)) {
        /* Modify through pointer - affects *ptr used in condition */
        *ptr = *ptr / 2;
        /* Additional operations */
        int temp = *ptr + 100;
        sink = sink + temp;
        *ptr = *ptr | 0x1;
    }
    if (ptr) sink = sink + *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x < y && (x & 0xF) != 0) {
            /* Modify 'x' which is used in condition */
            x = x + y;
            /* Additional statements */
            y = y ^ i;
            sink = sink + (x & 0xFF);
            x = x >> 1;
        } else {
            y = y + x;
        }
        /* Loop-carried dependency */
        x = x + i;
    }
    sink = sink + x + y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b) {
    /* Non-trivial condition using function call */
    if (check_cond(a, b) && (a & b) != 0) {
        /* Modify 'a' used in condition */
        a = get_value(a);
        /* Multiple assignments */
        b = b ^ a;
        sink = sink | b;
        a = a * 3 + 1;
        sink = sink + (a & 0xFF);
    }
    /* Use results */
    sink = sink + a - b;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Condition uses volatile globals */
    if (local_a > glob_c || local_b < glob_d) {
        /* Modify local variables derived from volatiles */
        local_a = local_a + glob_c;
        /* Additional operations */
        sink = sink ^ local_a;
        local_b = local_b * 2;
        sink = sink + local_b;
        /* Modify through function */
        modify(&local_a);
    }
    sink = sink + local_a + local_b;
}

/* Test 7: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_bb_then(int a, int b, int c) {
    /* Complex condition */
    if (a > b || b < c) {
        /* First modification */
        a = a ^ b;
        sink = sink + a;
        
        /* Second modification - different variable */
        b = b + c;
        sink = sink ^ b;
        
        /* Third modification - both variables */
        a = a * b;
        c = c - a;
        sink = sink | c;
    }
    sink = sink + a + b + c;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Array for pointer test */
    int data[4] = {arg1, arg2, arg3, arg4};
    
    printf("Starting if-conversion coverage tests...\n");
    
    /* Run all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multi_modification(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(&data[0], 50);
    result += sink;
    
    test_loop_nested(10);
    result += sink;
    
    test_complex_condition(arg1, arg2);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    test_multi_bb_then(arg1, arg2, arg3);
    result += sink;
    
    /* Final checksum */
    printf("Checksum: %d\n", result & 0xFFFF);
    printf("All tests completed.\n");
    
    return 0;
}
