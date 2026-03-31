/* Test program for if-conversion uncovered lines in ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Dummy functions to prevent inlining and simplification */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Complex condition to prevent simplification */
    if (a > b && (get_value(a) != 0)) {
        /* Modify variable used in condition */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b ^ 0x1234;
        sink += a * b;
    }
    
    /* Use results to prevent dead code elimination */
    glob_a = a;
    glob_b = b;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    int d = glob_d;
    
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (cond_check(a, c) != 0)) {
        /* Modify multiple variables from the condition */
        a = a * 2 + 1;
        b = b - 3;
        c = c ^ d;
        /* Additional computation */
        d = (d << 2) | 1;
        sink += a + b + c + d;
    }
    
    glob_a = a;
    glob_b = b;
    glob_c = c;
    glob_d = d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int x = glob_a;
    int y = glob_b;
    int *ptr = &x;
    
    /* Condition uses variable that will be indirectly modified */
    if (x > 0 && y < 100) {
        /* Indirect modification through pointer */
        *ptr = 42;
        /* Direct modification of another test variable */
        y = y * 2;
        /* Additional statements */
        int temp = x ^ y;
        sink += temp;
        modify(&y);
    }
    
    glob_a = x;
    glob_b = y;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int iterations = 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (a > b && (i % 2 == 0)) {
            /* Modify test expression variables */
            a = a + i;
            b = b - i;
            /* Additional computation to create real instructions */
            int product = a * b;
            sink += product;
            /* Call to prevent optimization */
            modify(&a);
        } else {
            /* Alternate path to preserve control flow */
            a = a ^ b;
            b = b + i;
        }
        
        /* Loop-carried dependency */
        a = get_value(a);
        b = get_value(b);
    }
    
    glob_a = a;
    glob_b = b;
}

/* Test 5: Complex condition with function calls */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    
    /* Complex condition that's hard to optimize away */
    if ((a > get_value(b)) && 
        (cond_check(a, c) != 0) && 
        (b < (c ^ 0xFFFF))) {
        /* Modify all variables used in condition */
        a = (a << 3) | 0x7;
        b = b + get_value(c);
        c = c * 2 - 1;
        
        /* Multiple real instructions in the block */
        int sum = a + b + c;
        sink += sum;
        sum = sum ^ 0xAAAAAAAA;
        sink += sum;
        
        /* Another modification */
        a = a ^ sum;
    }
    
    glob_a = a;
    glob_b = b;
    glob_c = c;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_cond(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    int regular_c = glob_c;
    
    /* Condition uses volatile variables */
    if (local_a > local_b && regular_c != 0) {
        /* Modify the volatile variable (harder to optimize) */
        local_a = local_a + 5;
        /* Also modify regular variable */
        regular_c = regular_c * 3;
        /* Additional computation */
        int diff = local_a - local_b;
        sink += diff * regular_c;
    }
    
    glob_a = local_a;
    glob_b = local_b;
    glob_c = regular_c;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting if-conversion test patterns...\n");
    
    /* Run all test cases multiple times with different initial values */
    for (int run = 0; run < 3; run++) {
        glob_a = 10 + run * 5;
        glob_b = 20 + run * 3;
        glob_c = 30 + run * 7;
        glob_d = 40 + run * 2;
        
        test_single_var_mod();
        checksum += sink;
        
        test_multi_var_mod();
        checksum += sink;
        
        test_indirect_mod();
        checksum += sink;
        
        test_loop_nested();
        checksum += sink;
        
        test_complex_cond();
        checksum += sink;
        
        test_volatile_cond();
        checksum += sink;
        
        /* Mix up values for next iteration */
        glob_a ^= 0x12345678;
        glob_b ^= 0x87654321;
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    printf("Final values: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
