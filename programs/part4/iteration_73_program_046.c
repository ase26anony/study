/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (glob_c != 0);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in the test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        glob_c = glob_c ^ a;
        sink = a * b;
        /* Another modification to ensure multiple instructions */
        a = a | 0x1;
    }
    sink = a + b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional with multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' from the test expression */
        a = a * 2 + 1;
        /* Additional arithmetic to create more instructions */
        b = b + (c >> 2);
        c = c ^ 0x12345678;
        /* Use volatile to prevent elimination */
        sink = a + b + c + d;
        /* Another modification to 'a' */
        a = get_value(a);
    }
    /* Use results to prevent dead code */
    glob_a = a;
    glob_b = b;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses dereferenced pointer */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through pointer - affects the value tested */
        *ptr = *ptr + 42;
        /* Additional operations to flesh out basic block */
        int temp = *ptr * 3;
        sink = temp;
        /* Another indirect modification */
        modify(ptr);
    }
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression with variables modified in loop */
        if (x > y && check_cond(x, y)) {
            /* Modify test expression variables */
            x = x + i;
            y = y - 1;
            /* Additional computation */
            int z = x * y;
            sink = z;
            /* Another modification */
            x = x ^ y;
        }
        /* Loop-carried dependency */
        y = y + (x >> 1);
    }
    
    glob_a = x;
    glob_b = y;
}

/* Test case 5: Complex arithmetic in test expression modification */
static void __attribute__((noinline, noipa)) test_complex_arithmetic(int a, int b) {
    /* Complex test expression */
    if ((a * b > 100) || ((a ^ b) < 50)) {
        /* Modify 'a' with complex arithmetic */
        a = (a << 3) | (b & 0xF);
        /* Multiple arithmetic operations */
        b = b + ((a * 3) / 2);
        a = a ^ b;
        /* Function call that might modify */
        modify(&a);
        /* Volatile access */
        sink = a;
    }
}

/* Test case 6: Mixed statements with function calls */
static void __attribute__((noinline, noipa)) test_mixed_statements(int a, int b) {
    /* Use function in condition */
    if (get_value(a) > b) {
        /* Modify 'a' */
        a = a + get_value(b);
        /* Multiple statements of different types */
        b = b * 2;
        sink = a;
        a = a | 0xFF;
        /* Another function call */
        modify(&a);
        /* Final assignment */
        glob_c = a + b;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int var1 = get_value(100);
    int var2 = get_value(200);
    int var3 = get_value(300);
    int var4 = get_value(400);
    
    /* Test 1: Single modification */
    test_single_modification(var1, var2);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(var1, var2, var3, var4);
    result += glob_a + glob_b;
    
    /* Test 3: Indirect modification */
    int ptr_val = 500;
    test_indirect_modification(&ptr_val, 400);
    result += ptr_val;
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    result += glob_a * glob_b;
    
    /* Test 5: Complex arithmetic */
    test_complex_arithmetic(var1, var3);
    result += sink;
    
    /* Test 6: Mixed statements */
    test_mixed_statements(var2, var4);
    result += glob_c;
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result);
    
    /* Additional test with volatile to ensure blocks aren't optimized away */
    volatile int v1 = 1, v2 = 2, v3 = 3;
    if (v1 > v2 || v3 != 0) {
        v1 = v1 + v3;
        v2 = v2 * 2;
        v3 = v1 ^ v2;
        sink = v1 + v2 + v3;
    }
    
    return 0;
}
