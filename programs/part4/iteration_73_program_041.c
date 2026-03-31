/* Test program for if-conversion uncovered lines in ifcvt.cc.gcov
 * Specifically targets lines 577-583 checking for modifications to test_expr
 * in the then basic block.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To consume results and prevent elimination */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;  /* Non-trivial computation */
}

static void __attribute__((noinline, noipa)) modify_value(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (a < b * 2);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + glob_d;
        /* Additional non-debug instructions */
        glob_d = glob_d ^ 0x1234;
        sink += a * 2;
    }
    sink += a + b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != 0)) {
        /* Modify multiple test expression variables */
        a = a * 3 + 1;
        b = b ^ 0xFF;
        c = get_value(c);
        /* Additional arithmetic to flesh out basic block */
        d = d << 2;
        sink += a + b + c + d;
    }
    sink += glob_b;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Test expression uses pointers */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        /* Indirect modification of value used in test expression */
        *ptr1 = *ptr1 + 100;
        /* Additional operations */
        *ptr2 = *ptr2 - 50;
        modify_value(ptr1);
        sink += *ptr1 * *ptr2;
    }
    sink += (ptr1 != 0);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (a > b && check_condition(a, b)) {
            /* Modify variables used in test expression */
            a = a + i;
            b = b - i;
            /* Additional computation to prevent optimization */
            sum += a * b;
            glob_c = glob_c ^ sum;
        }
        /* Loop-carried dependency */
        a = a ^ 0x1;
        b = b + 1;
    }
    sink += sum + a + b;
}

/* Test case 5: Complex arithmetic in then block */
static void __attribute__((noinline, noipa)) test_complex_arithmetic(int x, int y) {
    /* Test expression with arithmetic */
    if (x * 2 > y + 10 && glob_d != 0) {
        /* Modify 'x' which is used in test expression */
        x = (x << 3) | (x >> 5);  /* Rotate */
        /* Multiple arithmetic operations */
        y = y * 3 + x;
        int temp = x ^ y;
        sink += temp;
        /* Function call that might affect globals */
        modify_value(&glob_a);
    }
    sink += x - y;
}

/* Test case 6: Volatile access in conditional */
static void __attribute__((noinline, noipa)) test_volatile_access(int a, int b) {
    volatile int local_vol = 0;
    
    /* Test expression with volatile read */
    if (a > b && local_vol == 0) {
        /* Modify test expression variable */
        a = a + glob_c;
        /* Write to volatile to prevent reordering */
        local_vol = a;
        /* Additional operations */
        b = b * 2;
        sink += local_vol + b;
    }
    sink += a;
}

/* Test case 7: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_statement_then(int a, int b, int c) {
    /* Complex test expression */
    if (a > b && b < c && c != a) {
        /* Sequence of modifications to test expression variables */
        a = a * 2;          /* First modification */
        b = b + a;          /* Uses modified 'a' */
        c = c ^ b;          /* Uses modified 'b' */
        
        /* Additional independent operations */
        int t1 = a << 2;
        int t2 = b >> 1;
        sink += t1 + t2 + c;
        
        /* Another modification */
        a = a + t1;
        sink += a;
    }
    sink += b + c;
}

int main(void) {
    int result = 0;
    
    /* Initialize some local variables */
    int var1 = get_value(100);
    int var2 = get_value(200);
    int var3 = get_value(300);
    int var4 = get_value(400);
    
    /* Allocate memory for pointer test */
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    
    printf("Starting if-conversion test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification(var1, var2);
    result += sink;
    
    test_multiple_modifications(var1, var2, var3, var4);
    result += sink;
    
    test_indirect_modification(ptr1, ptr2);
    result += sink;
    
    test_loop_nested(5);
    result += sink;
    
    test_complex_arithmetic(var3, var4);
    result += sink;
    
    test_volatile_access(var1, var3);
    result += sink;
    
    test_multi_statement_then(var1, var2, var3);
    result += sink;
    
    /* Final output to prevent entire program elimination */
    printf("Result checksum: %d\n", result);
    printf("Global state: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return result != 0 ? 0 : 1;
}
