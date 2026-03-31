/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return a > b;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int x, int y) {
    /* Condition uses x and y */
    if (x > y && glob_c != 0) {
        /* Modify x which appears in the condition */
        x = x + 1;
        /* Additional non-debug instructions */
        glob_a = glob_a ^ 0x55;
        y = y * 3;
        /* Use volatile sink */
        sink = x + y;
    }
    /* Ensure the modified values are used */
    sink = x | y;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound condition using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both a and c from the condition */
        a = a + glob_b;
        /* Additional arithmetic */
        b = b ^ 0xFF;
        /* Modify another condition variable */
        c = c * 2 + 1;
        /* More instructions to flesh out the block */
        d = d - glob_a;
        sink = a + b + c + d;
    }
    /* Force use of modified values */
    glob_c = a ^ c;
}

/* Test 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Condition based on pointer values */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        /* Modify through pointer - affects *ptr1 which is in condition */
        *ptr1 = *ptr1 + 42;
        /* Additional modifications */
        *ptr2 = *ptr2 / 2;
        /* More instructions */
        int temp = *ptr1 ^ *ptr2;
        sink = temp;
    }
    /* Use results */
    if (ptr1) sink = *ptr1;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = get_value();
    int b = get_value();
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses loop-varying variables */
        if (a > b && i % 2 == 0) {
            /* Modify a which is in the condition */
            a = a + i;
            /* Additional computation */
            b = b ^ i;
            /* Use volatile to prevent elimination */
            sink = a * b;
        }
        /* Loop-carried dependency */
        a = a ^ 0x01;
        b = b + 1;
    }
    /* Final sink */
    sink = a + b;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y, int z) {
    /* Non-trivial condition */
    if (check_cond(x, y) && (z != 0) && (glob_a < glob_b)) {
        /* Modify x which appears in condition via function call */
        x = x * 2 + 5;
        /* Modify glob_a which is also in condition */
        glob_a = glob_a + 1;
        /* Additional sequence of instructions */
        y = y | 0xF0;
        z = z << 1;
        /* Multiple assignments to ensure basic block has content */
        int temp = x * y * z;
        sink = temp;
    }
    /* Use modified values */
    modify(&x);
    sink = x + y + z;
}

/* Test 6: Mixed modifications with volatile accesses */
static void __attribute__((noinline, noipa)) test_volatile_mix(int a, int b) {
    volatile int local_vol = 0;
    
    /* Condition using parameters and volatile */
    if (a > b || local_vol != 0) {
        /* Modify a from condition */
        a = a ^ b;
        /* Modify volatile - creates side effect */
        local_vol = a + b;
        /* Additional arithmetic */
        b = b * 3;
        /* More instructions */
        int c = a + b;
        int d = c * 2;
        sink = d;
    }
    /* Ensure volatile is read */
    sink = local_vol;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int val1 = get_value();
    int val2 = get_value();
    int val3 = get_value();
    int val4 = get_value();
    
    /* Test 1: Single modification */
    test_single_modification(val1, val2);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multi_modification(val1, val2, val3, val4);
    result += sink;
    
    /* Test 3: Indirect modification */
    int var1 = 100, var2 = 50;
    test_indirect_modification(&var1, &var2);
    result += sink + var1 + var2;
    
    /* Test 4: Loop nested */
    test_loop_nested(10);
    result += sink;
    
    /* Test 5: Complex condition */
    test_complex_condition(val1, val2, val3);
    result += sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix(val3, val4);
    result += sink;
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
