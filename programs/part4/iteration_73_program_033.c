/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int g_sink = 0;
int g_a = 10, g_b = 20, g_c = 30, g_d = 40;
volatile int g_vol = 0;

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) use_value(int x) {
    g_sink = x;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (g_vol & 1);
}

static int __attribute__((noinline, noipa)) modify(int x) {
    return x * 3 + 7;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b) {
        /* Modify 'a' which is used in the test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        int temp = a * 2;
        use_value(temp);
        a = a ^ 0xFF;
        g_sink = a;
    }
    use_value(a + b);
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound test expression */
    if ((a > b) && (c != d)) {
        /* Modify both 'a' and 'c' from the test expression */
        a = modify(a);
        /* Additional arithmetic */
        b = b + a;
        c = c * 2;
        /* Use volatile to prevent elimination */
        g_vol = a + c;
        /* More modifications */
        d = d ^ c;
        use_value(a + b + c + d);
    }
    g_sink = a + c;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through pointer - affects the value tested */
        *ptr = *ptr + 100;
        /* Additional operations */
        int temp = *ptr * 3;
        use_value(temp);
        *ptr = *ptr / 2;
        g_vol = *ptr;
    }
    if (ptr) use_value(*ptr);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = g_a;
    int y = g_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (x < y && (i % 3) != 0) {
            /* Modify 'x' which is in the test expression */
            x = x + i;
            /* Additional computation */
            y = y - (x & 0xF);
            /* Use volatile to create side effect */
            g_vol = x ^ y;
            /* More modifications */
            x = x | 0x1;
            use_value(x + y);
        }
        /* Loop-carried dependency */
        x = x ^ (y << 2);
        y = y + get_value(i);
    }
    g_sink = x + y;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Condition with function call */
    if (cond_check(a, b) && c > 0) {
        /* Modify 'a' which is used in cond_check */
        a = a * 2 + 5;
        /* Modify 'c' from the condition */
        c = c - 1;
        /* Sequence of assignments */
        int t1 = a << 3;
        int t2 = c ^ t1;
        a = a + t2;
        c = c * a;
        use_value(a + c);
        /* Additional modification */
        b = b ^ a;
        g_vol = b;
    }
    g_sink = a + b + c;
}

/* Test case 6: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_statement_then(int a, int b) {
    if (a != b) {
        /* First modification */
        a = a + b;
        /* Second modification with computation */
        b = b * 2;
        /* Third modification */
        a = a ^ b;
        /* Function call to prevent merging */
        use_value(a);
        /* Fourth modification */
        b = modify(b);
        /* Final use */
        g_sink = a - b;
    }
}

/* Test case 7: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_conditional(void) {
    volatile int local_vol = g_vol;
    int x = g_a;
    int y = g_b;
    
    /* Condition uses volatile */
    if (x > local_vol && y < 100) {
        /* Modify variables used in condition */
        x = x + local_vol;
        y = y * 2;
        /* Multiple statements */
        int z = x & y;
        x = x ^ z;
        y = y | x;
        g_vol = x + y;
        use_value(z);
    }
    g_sink = x + y;
}

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
    
    printf("Starting if-conversion test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification(arg1, arg2);
    result += g_sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += g_sink;
    
    test_indirect_modification(dynamic_ptr, 100);
    if (dynamic_ptr) result += *dynamic_ptr;
    
    test_loop_nested(10);
    result += g_sink;
    
    test_complex_condition(arg1, arg2, arg3);
    result += g_sink;
    
    test_multi_statement_then(arg4, arg1);
    result += g_sink;
    
    test_volatile_conditional();
    result += g_sink;
    
    /* Cleanup */
    if (dynamic_ptr) free(dynamic_ptr);
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
