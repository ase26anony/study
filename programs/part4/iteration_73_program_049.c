/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (a < 0);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent simplification */
    if (a > b && (a + b) % 7 != 0) {
        /* Modify the test expression variable 'a' */
        a = a * 2 + 1;                    /* First modification */
        sink += a;                        /* Use result to prevent elimination */
        a = get_value(a);                 /* Second modification via function */
        /* Additional non-debug instructions */
        int temp = a ^ b;
        sink += temp;
    }
    sink += a + b;  /* Ensure values are used */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d && (a + c) > 100)) {
        /* Modify multiple test expression variables */
        a = (a << 3) | 0x1F;              /* Modify 'a' */
        b = b ^ (b >> 4);                 /* Modify 'b' */
        c = c * 5 - 3;                    /* Modify 'c' */
        
        /* Additional arithmetic to flesh out the basic block */
        int t1 = a * b;
        int t2 = c + d;
        sink += t1 + t2;
        
        /* Call to opaque function */
        modify(&a);
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through the pointer - affects the test expression */
        *ptr = *ptr / 2;                  /* First modification */
        sink += *ptr;
        *ptr = get_value(*ptr);           /* Second modification */
        
        /* Additional operations */
        int temp = *ptr ^ threshold;
        sink += temp;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses loop-varying variables */
        if (a > b && (a + i) % 5 != 0) {
            /* Modify test expression variables */
            a = a + i + 1;                /* Modification depends on loop counter */
            b = b - (i % 3);
            
            /* Additional computation */
            sum += a * b;
            sink += sum & 0xFF;
        }
        /* Loop-carried dependency */
        a = (a + 1) & 0xFF;
        b = (b + i) & 0xFF;
    }
    sink += sum;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Condition computed via opaque function */
    if (check_condition(x, y) && (x ^ y) > 0) {
        /* Modify variables used in condition */
        x = (x * 3) >> 1;                 /* Arithmetic modification */
        y = y ^ 0x12345678;               /* Bitwise modification */
        
        /* Multiple statements in then block */
        int t1 = x + y;
        int t2 = x - y;
        sink += t1 * t2;
        
        /* Another modification */
        x = get_value(x);
    }
    sink += x + y;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int a = glob_a;
    volatile int v = glob_b;
    
    /* Condition uses volatile */
    if (a > v && glob_c != 0) {
        /* Modify both local and global test expression components */
        a = a + v;                        /* Uses volatile in computation */
        glob_c = glob_c - 1;              /* Modify global used in condition */
        
        /* Additional operations */
        sink += a * glob_c;
        a = a | 0x80000000;
    }
    sink += a + glob_c;
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize some test data */
    int arr[] = {100, 200, 300, 400};
    int *ptr = &arr[1];
    
    printf("Starting if-conversion modification tests...\n");
    
    /* Run test cases with different parameters */
    test_single_modification(glob_a, glob_b);
    result += sink;
    
    test_multiple_modifications(50, 60, 70, 80);
    result += sink;
    
    test_indirect_modification(ptr, 150);
    result += sink;
    
    test_loop_nested(10);
    result += sink;
    
    test_complex_condition(123, 456);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    /* Additional edge case: modification in both branches */
    {
        int x = 1000, y = 2000;
        if (x < y && (x & 1)) {
            x = x * 2;                    /* Modify in then block */
            y = y + x;
            sink += x ^ y;
        } else {
            x = x / 2;                    /* Different modification in else */
            sink += x;
        }
        result += x + y;
    }
    
    printf("Test result checksum: %d\n", result & 0xFFFF);
    printf("All tests completed.\n");
    
    return 0;
}
