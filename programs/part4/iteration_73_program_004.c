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
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify_value(int *ptr) {
    if (ptr) *ptr = (*ptr * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (a < b * 2);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Condition uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in the condition */
        a = a + glob_d;
        /* Additional non-debug instructions */
        glob_d = glob_d ^ 0x1234;
        sink = a * b;
        /* Another modification to ensure multiple instructions */
        a = a | 0xFF;
    }
    sink += a + b;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound condition using multiple variables */
    if ((a > b) || (c < d && glob_a != 0)) {
        /* Modify 'a' and 'c' which appear in the condition */
        a = get_value(a) + 1;
        /* Additional arithmetic to create more instructions */
        b = b * 3;
        c = (c << 2) | 0x1;
        /* Use volatile to prevent optimization */
        sink = a + b + c + d;
        /* Another modification to 'd' */
        d = d ^ a;
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Condition based on pointer values */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        /* Modify through pointer - affects the dereferenced value used in condition */
        *ptr1 = *ptr1 + 100;
        /* Additional operations */
        *ptr2 = *ptr2 * 2;
        sink = *ptr1 + *ptr2;
        /* More modifications */
        modify_value(ptr1);
    }
    if (ptr1) sink += *ptr1;
    if (ptr2) sink += *ptr2;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y || check_condition(x, y)) {
            /* Modify variables used in condition */
            x = x + i;
            y = y - (i & 0xF);
            /* Additional non-trivial computation */
            glob_c = glob_c ^ (x * y);
            sink = x + y + glob_c;
        }
        /* Loop-carried dependency */
        x = x ^ y;
        y = y + 1;
    }
    sink += x + y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Complex condition with multiple terms */
    if ((a > get_value(b)) && (c < glob_d || b != 0)) {
        /* Modify 'a', 'b', and 'c' - all used in condition */
        a = a * 2 + 1;
        b = b | 0xAA;
        /* Multiple arithmetic operations */
        c = (c << 3) + (a & 0xFF);
        sink = a + b + c;
        /* Additional modification */
        a = a ^ b ^ c;
        glob_d = glob_d + a;
    }
    sink += a + b + c;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_condition(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Condition uses volatile variables */
    if (local_a > local_b && local_a != 0) {
        /* Modify volatile variable used in condition */
        local_a = local_a + glob_c;
        /* Multiple operations */
        local_b = local_b * 3;
        sink = local_a + local_b;
        /* Another volatile modification */
        glob_a = local_a;
        glob_b = local_b;
    }
    sink += local_a + local_b;
}

/* Main function that exercises all test cases */
int main(void) {
    int arr1[2] = {100, 50};
    int arr2[2] = {75, 80};
    int result = 0;
    
    printf("Starting ifcvt test program...\n");
    
    /* Test 1: Single modification */
    test_single_modification(glob_a, glob_b);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(30, 40, 50, 60);
    result += sink;
    
    /* Test 3: Indirect modification */
    test_indirect_modification(&arr1[0], &arr2[0]);
    result += sink;
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    result += sink;
    
    /* Test 5: Complex condition */
    test_complex_condition(10, 20, 30);
    result += sink;
    
    /* Test 6: Volatile condition */
    test_volatile_condition();
    result += sink;
    
    /* Additional test with different parameters */
    test_single_modification(100, 200);
    result += sink;
    
    test_multiple_modifications(1, 2, 3, 4);
    result += sink;
    
    printf("Final checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
