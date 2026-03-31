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
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr = (*ptr * 3) / 2;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) | (a == b);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        glob_c = glob_c ^ a;
        sink = a * b;
        /* Another modification to ensure multiple instructions */
        a = a * 2 - 1;
    }
    sink += a;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' from test expression */
        a = get_value(a);
        /* Additional arithmetic to flesh out basic block */
        b = b + (a >> 3);
        c = c * 2 + 1;
        d = d ^ 0x1234;
        /* Use volatile to prevent elimination */
        sink = a + b + c + d;
    }
    /* Ensure result is used */
    glob_a = a;
    glob_b = b;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses pointer-derived value */
    if (ptr && *ptr > threshold) {
        /* Modify through pointer - affects *ptr used in condition */
        *ptr = *ptr / 2;
        /* Additional operations */
        modify(ptr);
        sink = *ptr + threshold;
        /* Another indirect modification */
        *ptr = *ptr ^ 0xFF;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (x > y && i % 3 != 0) {
            /* Modify 'x' which is used in condition */
            x = x + i;
            /* Additional statements */
            y = y ^ x;
            sink = x * y + i;
            /* Another modification */
            x = (x << 1) | 1;
        } else {
            y = y + i;
        }
        /* Loop-carried dependency */
        glob_c = x + y;
    }
    glob_a = x;
    glob_b = y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b) {
    /* Condition with function call */
    if (check_cond(a, b) && glob_d > 0) {
        /* Modify 'a' used in condition via function call */
        a = get_value(a);
        /* Multiple arithmetic operations */
        b = b + (a & 0xF);
        a = a * 3;
        b = b ^ a;
        /* Volatile write */
        sink = a - b;
        /* Additional modification */
        a = (a + b) >> 1;
    }
    glob_d = a + b;
}

/* Test 6: Modification in both then and else blocks */
static void __attribute__((noinline, noipa)) test_both_branches_modify(int a, int b) {
    /* Test expression */
    if (a != b && glob_c > glob_d) {
        /* Modify 'a' in then block */
        a = a * 2 + 1;
        b = b ^ 0xAA;
        sink = a + b;
        /* Additional arithmetic */
        a = a - b;
    } else {
        /* Also modify 'a' in else block */
        a = a / 2;
        b = b + 100;
        sink = a * b;
    }
    /* Use results */
    glob_a = a;
    glob_b = b;
}

/* Test 7: Pointer arithmetic in condition */
static void __attribute__((noinline, noipa)) test_pointer_arithmetic(int *arr, int size) {
    int *p = arr;
    int *end = arr + size;
    
    while (p < end) {
        /* Condition using pointer */
        if (p != NULL && *p > 0) {
            /* Modify through pointer */
            *p = *p * 2;
            /* Additional operations */
            modify(p);
            sink += *p;
            /* Pointer arithmetic */
            p++;
        } else {
            *p = -1;
            p++;
        }
    }
}

int main(void) {
    int arr[10];
    int i, result = 0;
    
    /* Initialize array */
    for (i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    
    /* Run all test cases */
    test_single_modification(5, 3);
    test_multiple_modifications(15, 10, 25, 30);
    test_indirect_modification(&arr[3], 20);
    test_loop_nested(5);
    test_complex_condition(7, 12);
    test_both_branches_modify(9, 9);
    test_pointer_arithmetic(arr, 10);
    
    /* Aggregate results */
    for (i = 0; i < 10; i++) {
        result += arr[i];
    }
    result += glob_a + glob_b + glob_c + glob_d + sink;
    
    printf("Result checksum: %d\n", result);
    printf("Test completed successfully.\n");
    
    return 0;
}
