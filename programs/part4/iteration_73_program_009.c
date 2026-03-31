/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink for side effects */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr = (*ptr * 3) + 7;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int x, int y) {
    /* Test expression uses x and y */
    if (x > y && x != 0) {
        /* Modify x which appears in test expression */
        x = x + (y * 2);
        /* Additional non-debug instructions */
        sink = x | y;
        x = x ^ 0x1234;
        /* Call to prevent optimization */
        modify(&x);
    }
    sink += x;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Complex test expression */
    if ((a > b || c < d) && (a + c) != (b + d)) {
        /* Modify multiple test expression variables */
        a = a * 2 + 1;
        b = b - c;
        /* Additional arithmetic */
        c = (c << 3) | (d & 0xF);
        /* Volatile access to prevent elimination */
        sink = a + b + c + d;
        /* Another modification */
        d = d ^ a;
    }
    /* Use results to prevent dead code */
    glob_a = a;
    glob_b = b;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Test expression uses pointer values */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        /* Modify through pointer - affects what test expression checks */
        *ptr1 = *ptr1 / 2;
        /* Additional operations */
        *ptr2 = (*ptr2 + *ptr1) & 0xFF;
        /* Non-pointer operation to flesh out block */
        sink = (*ptr1 << 8) | *ptr2;
        /* Another indirect modification */
        *ptr1 = get_value(*ptr1);
    }
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (x > y && (x - y) < 100) {
            /* Modify test expression variables */
            x = x + (y >> 1);
            y = y + (i & 3);
            /* Additional computation */
            acc += x * y;
            /* Function call with side effect */
            modify(&x);
        } else {
            y = y - (x >> 2);
        }
        /* Loop-carried dependency */
        x = (x + i) & 0x7FFF;
    }
    sink += acc;
    glob_c = x;
    glob_d = y;
}

/* Test 5: Complex boolean expression with partial modification */
static void __attribute__((noinline, noipa)) test_complex_boolean(int p, int q, int r, int s) {
    /* Multi-part boolean expression */
    if ((p > q) && (r < s) && ((p ^ q) != (r ^ s))) {
        /* Modify only some of the test expression variables */
        p = p * 3 - q;
        r = r ^ s;
        /* Sequence of assignments */
        q = q + (p & 0xFF);
        s = s | 0x8000;
        /* Arithmetic with test variables */
        sink = (p << 16) | (q << 8) | r;
        /* Another modification */
        p = get_value(p);
    }
    /* Use modified values */
    glob_a ^= p;
    glob_b ^= q;
}

/* Test 6: Function call in condition with modification */
static void __attribute__((noinline, noipa)) test_func_cond_mod(int a, int b) {
    /* Condition uses function call */
    if (check_cond(a, b) && (a + b) > 0) {
        /* Modify variables used in condition */
        a = a ^ b;
        b = b + get_value(a);
        /* Multiple assignments */
        int t = a * b;
        sink = t % 256;
        a = (a << 1) | (b & 1);
    }
    glob_c = a + b;
}

/* Main driver */
int main(void) {
    int result = 0;
    
    /* Initialize some local variables */
    int var1 = 100;
    int var2 = 50;
    int var3 = 75;
    int var4 = 125;
    
    /* Allocate memory for pointer test */
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    
    printf("Starting if-conversion modification tests...\n");
    
    /* Run test cases */
    test_single_modification(var1, var2);
    result += sink;
    
    test_multi_modification(var1, var2, var3, var4);
    result += glob_a + glob_b;
    
    test_indirect_modification(ptr1, ptr2);
    result += *ptr1 + *ptr2;
    
    test_loop_nested(10);
    result += glob_c + glob_d;
    
    test_complex_boolean(var1, var2, var3, var4);
    result += glob_a ^ glob_b;
    
    test_func_cond_mod(var3, var4);
    result += glob_c;
    
    /* Final sink to prevent optimization */
    volatile int final_sink = result;
    printf("Test result checksum: %d\n", final_sink);
    
    return (final_sink != 0) ? 0 : 1;
}
