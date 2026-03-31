/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify_value(int *x) {
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent constant folding */
    if (a > b && glob_c != 0) {
        /* Modify variable used in condition */
        a = a + glob_d;  // This modifies 'a' from test expression
        
        /* Additional non-debug instructions */
        glob_c = glob_c ^ 0x1234;
        sink = a * b;
        
        /* Another modification to ensure multiple instructions */
        a = get_value(a);
        sink += a;
    }
    /* Use result to prevent dead code elimination */
    volatile int local_sink = a + b;
    (void)local_sink;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != glob_b)) {
        /* Modify multiple variables from the condition */
        a = a * 2 + 1;      // Modifies 'a'
        c = c ^ d;          // Modifies 'c'
        glob_b = glob_b + 1; // Modifies glob_b used in condition
        
        /* Additional arithmetic to create more instructions */
        int temp = a * c;
        temp = temp >> 3;
        sink = temp + d;
        
        /* Function call that might modify */
        modify_value(&b);
    }
    
    /* Ensure side effects are visible */
    volatile int local_sink = a + b + c + d;
    (void)local_sink;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition based on pointer and value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification through pointer */
        *ptr = *ptr + threshold;  // Modifies what ptr points to
        
        /* Additional operations to create basic block body */
        int temp = *ptr;
        temp = temp * 3;
        sink = temp ^ 0xABCD;
        
        /* Modify threshold as well */
        threshold = get_value(threshold);
        sink += threshold;
    }
    
    /* Use variables */
    volatile int local_sink = threshold;
    if (ptr) local_sink += *ptr;
    (void)local_sink;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition that uses variables modified in loop */
        if (a > b && (i % 3) != 0) {
            /* Modify variables used in condition */
            a = a + i;      // Modifies 'a' from test expression
            b = b - 1;      // Modifies 'b' from test expression
            
            /* Additional non-trivial operations */
            int temp = a * b;
            temp = temp & 0xFF;
            sum += temp;
            
            /* Function call to prevent optimization */
            modify_value(&a);
        } else {
            /* Else branch to preserve control flow */
            a = a ^ b;
            sum -= i;
        }
        
        /* Loop-carried dependency */
        b = b + (sum & 1);
    }
    
    sink = sum;
    volatile int local_sink = a + b;
    (void)local_sink;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Condition with function call */
    if (check_condition(x, y) && (x + y) > glob_c) {
        /* Modify variables used in condition */
        x = x * 3;          // Modifies 'x'
        glob_c = glob_c - 1; // Modifies 'glob_c'
        
        /* Multiple instructions in basic block */
        int temp1 = x >> 2;
        int temp2 = y << 3;
        sink = temp1 ^ temp2;
        
        /* Another modification */
        y = get_value(y);
        sink += y;
        
        /* More arithmetic */
        x = x + temp1 - temp2;
        sink ^= x;
    }
    
    volatile int local_sink = x + y;
    (void)local_sink;
}

/* Test 6: Multiple basic blocks with modifications */
static void __attribute__((noinline, noipa)) test_multi_block(int a, int b, int c) {
    /* First condition */
    if (a > b) {
        a = a + c;      // Modifies 'a'
        b = b * 2;      // Modifies 'b'
        sink = a ^ b;
    }
    
    /* Second condition - uses modified variables */
    if (b < c || a != 0) {
        c = c - a;      // Modifies 'c'
        a = a | 0xF;    // Modifies 'a' again
        sink += c * a;
    }
    
    /* Third condition - compound */
    if ((a & 1) && (b > 0) && (c != 0)) {
        /* Modify all three */
        a = a >> 1;
        b = b << 1;
        c = c ^ 0xAA;
        sink = sink ^ (a + b + c);
    }
    
    volatile int local_sink = a + b + c;
    (void)local_sink;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Run all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    int test_val = 50;
    test_indirect_modification(&test_val, 30);
    result += sink + test_val;
    
    test_loop_nested(10);
    result += sink;
    
    test_complex_condition(arg1, arg2);
    result += sink;
    
    test_multi_block(arg1, arg2, arg3);
    result += sink;
    
    /* Final output to prevent complete optimization */
    printf("Result checksum: %d\n", result & 0xFF);
    
    return 0;
}
