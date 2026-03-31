/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p check in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x1234;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3) / 2;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ 0x1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* if (a > b) modifies a in then block */
    if (a > b) {
        a = a + 1;           /* Direct modification of test expression variable */
        b = b * 2;           /* Additional statement to flesh out basic block */
        sink = a + b;        /* Use volatile sink to prevent elimination */
        a = a ^ 0xABCD;      /* Another modification of 'a' */
    }
    
    sink = a + b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    int d = glob_d;
    
    /* Compound conditional using multiple variables */
    if ((a > b) && (c != d)) {
        a = a * 3 + 1;       /* Modify 'a' from first part of condition */
        c = c ^ d;           /* Modify 'c' from second part of condition */
        b = b >> 1;          /* Additional modification */
        sink = a + b + c + d;
        
        /* Multiple statements to ensure basic block has content */
        a = get_value(a);
        c = c | 0xFF;
    }
    
    sink = a + b + c + d;
}

/* Test case 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int x = glob_a;
    int y = glob_b;
    int *ptr = &x;
    
    /* Test expression uses x, then block modifies through pointer */
    if (x > y && ptr != NULL) {
        *ptr = 42;           /* Indirect modification of x */
        y = y + *ptr;        /* Additional computation */
        sink = x + y;
        
        /* More operations to flesh out the block */
        modify(&x);
        y = y << 2;
    }
    
    sink = x + y;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Condition uses variables modified in loop */
        if (a > b && i % 2 == 0) {
            a = a + i;       /* Modify test expression variable */
            b = b - 1;       /* Modify other test expression variable */
            sum += a * b;
            
            /* Additional statements */
            a = a ^ i;
            b = b | 0x1;
        }
        
        /* Loop-carried dependency prevents optimization */
        sink = a + b + sum;
    }
    
    sink = sum;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    
    /* Condition computed via opaque function */
    if (check_cond(a, b) && c > 0) {
        a = a * 2 + 5;       /* Modify 'a' used in check_cond */
        c = c - 1;           /* Modify 'c' used in condition */
        sink = a + b + c;
        
        /* Sequence of operations */
        a = a >> 1;
        b = b + a;
        c = c * 3;
    }
    
    sink = a + b + c;
}

/* Test case 6: Modification via array indexing */
static void __attribute__((noinline, noipa)) test_array_mod(void) {
    int arr[4] = {glob_a, glob_b, glob_c, glob_d};
    int idx = glob_a % 4;
    
    if (arr[idx] > glob_b && idx < 3) {
        arr[idx] = arr[idx] * 2;     /* Modify array element used in condition */
        idx = idx + 1;               /* Modify index variable */
        sink = arr[0] + arr[1] + arr[2] + arr[3];
        
        /* More operations */
        arr[idx] = get_value(arr[idx]);
        modify(&arr[0]);
    }
    
    sink = arr[0] + arr[1];
}

/* Test case 7: Multiple modifications in sequence */
static void __attribute__((noinline, noipa)) test_multi_mod_sequence(void) {
    int x = glob_a;
    int y = glob_b;
    int z = glob_c;
    
    if (x > y || z < 100) {
        /* First modification */
        x = x + y;
        sink = x;
        
        /* Second modification */
        y = y * 2;
        sink = y;
        
        /* Third modification */
        z = z ^ x;
        sink = z;
        
        /* Additional arithmetic */
        x = x | y;
        y = y & z;
        z = z + x;
    }
    
    sink = x + y + z;
}

int main(void) {
    int result = 0;
    
    printf("Testing if-conversion modification detection...\n");
    
    /* Execute all test cases */
    test_single_var_mod();
    result += sink;
    
    test_multi_var_mod();
    result += sink;
    
    test_indirect_mod();
    result += sink;
    
    test_loop_nested();
    result += sink;
    
    test_complex_cond();
    result += sink;
    
    test_array_mod();
    result += sink;
    
    test_multi_mod_sequence();
    result += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    printf("All tests completed.\n");
    
    return 0;
}
