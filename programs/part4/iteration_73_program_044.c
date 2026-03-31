/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink to prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *p) {
    if (p) *p += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) & 1;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b) {
        /* Modify 'a' which is used in the test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        sink = a * 2;
        a = a ^ 0x1234;
        sink += a;
    }
    sink += a + b;
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' from the test expression */
        a = a * 2 + 1;
        /* Additional arithmetic */
        b = b + (a >> 3);
        c = c - 5;
        /* More operations to create multiple instructions */
        sink = a + b + c + d;
        d = d ^ a;
        sink += d;
    }
    sink += a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr && *ptr > threshold) {
        /* Modify through the pointer - affects the test expression */
        *ptr = *ptr / 2;
        /* Additional operations */
        int temp = *ptr + 100;
        sink = temp;
        *ptr = *ptr | 0xFF;
    }
    if (ptr) sink += *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (x > y && (x + y) < 1000) {
            /* Modify variables used in test expression */
            x = x + y;
            y = y * 2 - 1;
            /* Additional arithmetic to create more instructions */
            int z = x * y;
            sink += z;
            x = x ^ (y << 2);
        } else {
            x = x - 1;
            y = y + 2;
        }
        /* Loop-carried dependency */
        sink = sink ^ (x + y);
    }
    glob_a = x;
    glob_b = y;
}

/* Test 5: Complex test with function calls */
static void __attribute__((noinline, noipa)) test_function_based(int a, int b) {
    /* Test expression computed via opaque function */
    if (check_cond(a, b)) {
        /* Modify 'a' which was used in the condition */
        a = get_value(a);
        /* Call function that might have side effects */
        modify(&a);
        /* More arithmetic */
        b = b + (a % 256);
        sink = a * b;
        a = a >> 4;
    }
    sink += a - b;
}

/* Test 6: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_based(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Test expression uses volatile-derived values */
    if (local_a > local_b && glob_c != glob_d) {
        /* Modify variables used in test expression */
        local_a = local_a + glob_c;
        local_b = local_b - glob_d;
        /* Operations with volatile */
        sink = local_a * local_b;
        glob_c = glob_c ^ local_a;
        local_a = local_a | 0xAA;
    }
    glob_a = local_a;
    glob_b = local_b;
}

/* Test 7: Multiple basic blocks with modifications */
static void __attribute__((noinline, noipa)) test_multi_block(int a, int b, int c) {
    /* More complex conditional structure */
    if (a > 0) {
        if (b < c) {
            /* Modify 'a' from outer test expression */
            a = a * 3;
            b = b + a;
            sink = a + b + c;
            /* Additional modification */
            a = a >> 1;
            c = c ^ a;
        }
        /* 'a' is modified in the then block above */
        sink += a * 100;
    }
    sink += b + c;
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
    
    printf("Starting if-conversion modification tests...\n");
    
    /* Run all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(&data[0], 50);
    result += sink;
    
    test_loop_nested(10);
    result += sink;
    
    test_function_based(arg3, arg4);
    result += sink;
    
    test_volatile_based();
    result += sink;
    
    test_multi_block(arg1, arg2, arg3);
    result += sink;
    
    /* Final checksum */
    volatile int final_sink = result;
    printf("Test checksum: %d\n", final_sink);
    printf("All tests completed.\n");
    
    return 0;
}
