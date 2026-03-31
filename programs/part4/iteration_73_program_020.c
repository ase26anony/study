/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p check in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink; /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    *ptr = (*ptr * 3) / 2;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Condition uses 'a' */
    if (a > b) {
        /* Modify 'a' which is used in the condition */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b ^ 0xFF;
        sink = a * b; /* Use volatile sink to prevent elimination */
    }
    sink = a; /* Ensure value is used */
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound condition using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both variables from different parts of condition */
        a = a * 2 + 1;
        c = c - 5;
        /* Additional arithmetic to flesh out basic block */
        d = (d << 2) | 1;
        sink = a + c + d;
    }
    sink = b; /* Use b to prevent elimination */
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition uses value pointed by ptr */
    if (ptr && (*ptr > threshold)) {
        /* Modify the value through pointer */
        *ptr = *ptr * 2;
        /* Additional operations */
        int temp = *ptr ^ 0x1234;
        sink = temp;
        /* Another modification */
        *ptr = *ptr + 1;
    }
    if (ptr) sink = *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y && (x % 3) == 0) {
            /* Modify test expression variables */
            x = x + y;
            y = y ^ i;
            /* Additional computation */
            int z = x * y - i;
            sink = z;
        } else {
            /* Else branch to maintain control flow complexity */
            x = x - 1;
        }
        /* Loop-carried dependency */
        y = y + (i & 1);
    }
    sink = x + y;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Non-trivial condition with function call */
    if (check_cond(a, b) && (c != 0)) {
        /* Modify variables used in condition */
        a = get_value(a);
        c = c >> 1;
        /* Multiple statements in then block */
        b = b * 3;
        int temp = (a & b) | c;
        sink = temp;
        /* Another modification */
        a = a ^ b;
    }
    sink = a + b + c;
}

/* Test case 6: Modification through function call */
static void __attribute__((noinline, noipa)) test_modify_via_call(int a, int b) {
    /* Condition using variables */
    if (a > 0 && b < 100) {
        /* Modify via function call */
        modify(&a);
        /* Direct modification */
        b = b + a;
        /* More statements */
        a = a | 0x1;
        sink = a * b;
    }
    sink = a - b;
}

/* Test case 7: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Condition using volatile-derived values */
    if (local_a > local_b && glob_c > 0) {
        /* Modify variables */
        local_a = local_a * glob_c;
        local_b = local_b + 1;
        /* Access volatile to prevent reordering */
        sink = glob_a;
        /* More modifications */
        local_a = local_a ^ local_b;
    }
    sink = local_a + local_b;
}

/* Test case 8: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_statement_then(int a, int b, int c) {
    /* Complex condition */
    if ((a > b) || (b < c) || ((a + b) > 100)) {
        /* Sequence of modifications */
        a = a + b;
        b = b * 2;
        c = c - a;
        
        /* Intermediate computation */
        int t1 = a ^ b;
        int t2 = c & 0xFF;
        
        /* Modify condition variable again */
        a = t1 + t2;
        
        /* Use results */
        sink = a + b + c;
    }
    sink = a;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Array for pointer test */
    int data = 50;
    int *ptr = &data;
    
    printf("Starting if-conversion test patterns...\n");
    
    /* Execute all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(ptr, 25);
    result += sink;
    
    test_loop_nested(5);
    result += sink;
    
    test_complex_condition(arg1, arg2, arg3);
    result += sink;
    
    test_modify_via_call(arg1, arg2);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    test_multi_statement_then(arg1, arg2, arg3);
    result += sink;
    
    /* Final checksum */
    printf("Result checksum: %d\n", result);
    printf("All test patterns executed.\n");
    
    return 0;
}
