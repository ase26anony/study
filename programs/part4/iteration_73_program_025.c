/* Test program for if-conversion uncovered lines in ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (glob_c & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in test expression */
        a = a + glob_d;  // Direct modification
        a = a * 2;       // Additional arithmetic
        glob_a = a;      // Store to volatile global
        sink += a;       // Use result to prevent elimination
    }
    sink += b;  // Ensure b is used
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' from test expression */
        a = a ^ 0x1234;      // Bitwise operation on a
        c = c * 3 + 1;       // Arithmetic on c
        b = b >> 2;          // Also modify b (appears in condition)
        
        /* Additional statements to flesh out basic block */
        int temp = a * c;
        sink += temp;
        glob_b = b + c;
    }
    /* Use variables outside to prevent dead code */
    sink += d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses pointer and value */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through pointer - affects *ptr used in condition */
        *ptr = *ptr + glob_a;
        *ptr = *ptr & 0xFF;
        
        /* Additional operations */
        int val = *ptr;
        sink += val;
        modify(ptr);  // Call function that might modify
    }
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y && (i % 3) != 0) {
            /* Modify x which is used in test expression */
            x = x + i;
            x = x ^ y;  // Additional operation
            
            /* Modify y as well */
            y = y - 1;
            
            /* Use volatile to prevent elimination */
            sink += x * y;
        } else {
            /* Else branch to maintain control flow */
            x = x - 1;
        }
        
        /* Loop-carried dependency */
        y = get_value(y);
    }
    
    glob_c = x + y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b) {
    /* Condition with function call */
    if (check_cond(a, b) && (glob_d > 0)) {
        /* Modify 'a' used in check_cond() */
        a = a * a + b;
        a = a % 100;
        
        /* Also modify global used in condition */
        glob_d = glob_d - 1;
        
        /* Multiple statements in basic block */
        for (int i = 0; i < 3; i++) {
            a = a + i;
            sink += a;
        }
    }
}

/* Test 6: Modification in both then and else with external side effects */
static void __attribute__((noinline, noipa)) test_both_branches_modify(int a, int b) {
    volatile int local_sink = 0;
    
    if (a != b) {
        /* Modify a in then block */
        a = a + glob_c;
        a = a | 0x1;  // Ensure modification
        local_sink = a;
        
        /* Call opaque function */
        modify(&glob_a);
    } else {
        /* Modify b in else block */
        b = b * 2;
        local_sink = b;
    }
    
    /* Use results */
    sink += local_sink;
}

/* Main function that exercises all test cases */
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
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    test_indirect_modification(dynamic_ptr, 100);
    result += sink;
    
    test_loop_nested(10);
    result += sink;
    
    test_complex_condition(arg1, arg3);
    result += sink;
    
    test_both_branches_modify(arg2, arg4);
    result += sink;
    
    /* Clean up */
    if (dynamic_ptr) {
        free(dynamic_ptr);
    }
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    printf("Global state: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
