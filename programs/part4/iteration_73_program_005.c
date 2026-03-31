/* Test program for ifcvt.cc uncovered lines 577-583 */
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

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) modifies a in then block */
    if (a > b) {
        /* Multiple statements including modification of test variable */
        a = a + 1;           /* Modifies test expression variable */
        sink = a * 2;        /* Non-debug instruction */
        a = a ^ 0x1234;      /* Another modification */
        sink += glob_c;      /* External side effect */
    }
    sink += a;  /* Use result to prevent elimination */
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound condition with multiple variables */
    if ((a > b) && (c != d)) {
        /* Modify both variables used in condition */
        a = a * 3 + 7;       /* Modifies first test variable */
        c = c ^ d;           /* Modifies second test variable */
        b = b + 1;           /* Additional modification */
        sink = a + c + b;    /* Use results */
        
        /* More instructions to ensure basic block has content */
        d = (d << 3) | (d >> 29);
        sink += d;
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr && (*ptr > threshold)) {
        /* Modify through pointer - affects test expression */
        *ptr = *ptr / 2;      /* Modifies what was tested */
        int temp = *ptr + 5;
        *ptr = temp ^ 0xFF;
        sink = *ptr;
        
        /* Additional arithmetic */
        threshold = threshold * 2 + 1;
        sink += threshold;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y && (x & 1)) {
            /* Modify test expression variables */
            x = x + y;        /* Modifies x used in condition */
            y = y ^ i;        /* Modifies y used in condition */
            sink = x * y;     /* External side effect */
            
            /* More complex computation */
            x = (x << 2) | (x >> 30);
            y = y * 3 + 1;
        }
        /* Loop-carried dependency */
        x = x + i;
        y = y - 1;
    }
    sink += x + y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_function_condition(int a, int b) {
    /* Condition computed via non-inlineable function */
    if (check_cond(a, b)) {
        /* Modify variables used in function's computation */
        a = get_value(a);     /* Modifies a (used in check_cond) */
        b = b * 7 + 3;        /* Modifies b (used in check_cond) */
        sink = a + b;
        
        /* Additional statements */
        int c = a ^ b;
        a = a + c;
        b = b - c;
        sink += a * b;
    }
    sink += a - b;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Condition uses volatile globals */
    if (local_a > glob_c && local_b < glob_d) {
        /* Modify local variables used in condition */
        local_a = glob_c + 1;    /* Uses volatile in computation */
        local_b = glob_d - 1;    /* Uses volatile in computation */
        sink = local_a * local_b;
        
        /* More operations */
        local_a = local_a ^ local_b;
        local_b = local_b * 2;
        sink += local_a + local_b;
        
        /* Force memory barrier */
        asm volatile("" : : : "memory");
    }
    sink += local_a + local_b;
}

/* Main function that exercises all test cases */
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
    
    printf("Starting ifcvt test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    if (dynamic_ptr) {
        test_indirect_modification(dynamic_ptr, 100);
        result += sink;
    }
    
    test_loop_nested(5);
    result += sink;
    
    test_function_condition(arg1, arg2);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    /* Clean up */
    if (dynamic_ptr) {
        free(dynamic_ptr);
    }
    
    printf("Test result checksum: %d\n", result);
    printf("All tests completed.\n");
    
    return 0;
}
