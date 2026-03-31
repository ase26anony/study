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

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) { a = a + 1; ... } */
    if (a > b) {
        /* Multiple statements including modification of test variable */
        a = a + 1;                    /* Modifies test expression variable */
        int temp = b * 3;             /* Additional computation */
        sink = temp ^ a;              /* Volatile sink to prevent removal */
        a = a | 0x1;                  /* Another modification of 'a' */
        sink += a;                    /* Ensure side effect */
    }
    sink += a + b;  /* Use results */
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* if (a > b && c != d) { modify both a and c } */
    if (a > b && c != d) {
        a = get_value(a);             /* Modify 'a' via function call */
        c = c * 2 + 1;                /* Direct modification of 'c' */
        int t = b ^ d;                /* Additional computation */
        sink = t;
        a = a >> 1;                   /* Another modification of 'a' */
        c = c & 0xFF;                 /* Another modification of 'c' */
        sink += a + c;
    }
    sink += a + b + c + d;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* if (ptr && *ptr > threshold) { *ptr = 42; ... } */
    if (ptr && *ptr > threshold) {
        *ptr = 42;                    /* Indirect modification through pointer */
        int temp = *ptr * 2;
        sink = temp;
        *ptr = *ptr + threshold;      /* Another indirect modification */
        sink += *ptr;
    }
    if (ptr) sink += *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* if (a < b + i) { modify a, creating loop-carried dependency } */
        if (a < b + i) {
            a = a + i;                /* Modifies test variable, creates dependency */
            int t = b * i;
            sink = t ^ a;
            a = a | 0x80000000;       /* Another modification */
            sink += a;
        }
        b = b ^ (i + 1);              /* Also modify b in loop */
        sink += b;
    }
    glob_a = a;  /* Store back to global */
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* if (cond_check(a, b) && a != c) { modify a and c } */
    if (cond_check(a, b) && a != c) {
        a = a * 3;                    /* Modify 'a' */
        c = get_value(c);             /* Modify 'c' via function */
        int temp = a ^ c;
        sink = temp;
        a = a + c;                    /* Another modification using both */
        sink += a;
    }
    sink += a + b + c;
}

/* Test case 6: Modification in both then and else blocks */
static void __attribute__((noinline, noipa)) test_both_branches_modify(int a, int b) {
    /* if (a > b) { a = ... } else { b = ... } - both modify test vars */
    if (a > b) {
        a = a * 2 + 1;                /* Modify 'a' in then block */
        int t = b << 2;
        sink = t;
        a = a ^ 0x12345678;           /* Another modification */
        sink += a;
    } else {
        b = b * 3 - 1;                /* Modify 'b' in else block */
        int t = a >> 1;
        sink = t;
        b = b & 0x7FFFFFFF;           /* Another modification */
        sink += b;
    }
    sink += a + b;
}

/* Test case 7: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_condition(void) {
    int a = glob_a;
    volatile int* vptr = &glob_b;
    
    /* if (a > *vptr) { modify a } */
    if (a > *vptr) {
        a = a + *vptr;                /* Modification using volatile read */
        int temp = a * 2;
        sink = temp;
        a = a % 100;                  /* Another modification */
        sink += a;
    }
    glob_a = a;
}

/* Main function that runs all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Run test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    int test_val = 50;
    int *ptr = &test_val;
    test_indirect_modification(ptr, 25);
    result += sink + test_val;
    
    test_loop_nested(5);
    result += sink + glob_a;
    
    test_complex_condition(arg1, arg2, arg3);
    result += sink;
    
    test_both_branches_modify(arg2, arg1);
    result += sink;
    
    test_volatile_condition();
    result += sink + glob_a + glob_b;
    
    /* Print checksum to ensure execution */
    printf("Test checksum: %d\n", result & 0xFF);
    
    return 0;
}
