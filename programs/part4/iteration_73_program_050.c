/* Test program for ifcvt.cc uncovered lines 577-583 */
/* Compile with: gcc -O2 -fno-if-conversion -fno-if-conversion2 -fdump-rtl-ifcvt -o test_ifcvt test_ifcvt.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
int glob_a = 10, glob_b = 20, glob_c = 30, glob_d = 40;
volatile int sink = 0;  /* Prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return a > b;
}

static int __attribute__((noinline, noipa)) modify_val(int x) {
    return x * 2 + 1;
}

static void __attribute__((noinline, noipa)) dummy_op(int *x) {
    *x ^= 0x55AA55AA;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) { a = modify_val(a); ... } */
    if (a > b) {
        /* Multiple statements including modification of test variable */
        a = modify_val(a);           /* Modifies 'a' from test expression */
        int temp = a * 3;            /* Additional computation */
        sink = temp;                 /* Volatile sink to prevent removal */
        dummy_op(&a);                /* Another modification */
        glob_a = a;                  /* Global side effect */
    }
    sink = a + b;  /* Ensure value is used */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* if (a > b || c < d) { modify both a and c } */
    if (a > b || c < d) {
        /* Modify multiple test expression variables */
        a = a + 1;                   /* Modifies 'a' from first part of condition */
        c = c * 2;                   /* Modifies 'c' from second part of condition */
        int sum = a + b + c + d;     /* Additional computation */
        sink = sum;                  /* Volatile sink */
        glob_b = b ^ c;              /* Global side effect */
    }
    sink = a + c;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* if (ptr && *ptr > threshold) { *ptr = 42; } */
    if (ptr && *ptr > threshold) {
        /* Modify through pointer - affects *ptr which is in test expression */
        *ptr = 42;                   /* Modifies dereferenced value from condition */
        int temp = *ptr + threshold;
        sink = temp;                 /* Volatile sink */
        dummy_op(ptr);               /* Another modification */
    }
    if (ptr) sink = *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* if (x < y) { modify x, which is in condition } */
        if (x < y) {
            x = x + i;               /* Modifies 'x' from test expression */
            y = y - 1;               /* Also modifies 'y' */
            sink = x * y;            /* Volatile sink prevents elimination */
        } else {
            x = x - 1;
        }
        
        /* Additional computation to create more instructions in block */
        int temp = x ^ y;
        dummy_op(&temp);
        sink = temp;
    }
    glob_c = x + y;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* if (check_cond(a, b) && c != 0) { modify a and c } */
    if (check_cond(a, b) && c != 0) {
        a = modify_val(a);           /* Modifies 'a' from first part of condition */
        c = c >> 1;                  /* Modifies 'c' from second part of condition */
        
        /* Multiple additional statements */
        int t1 = a * b;
        int t2 = c ^ 0xFF;
        sink = t1 + t2;
        
        /* Another modification of test variable */
        a = a | 0x01;
        glob_d = a + c;
    }
    sink = a + b + c;
}

/* Test 6: Modification with volatile access in then block */
static void __attribute__((noinline, noipa)) test_volatile_mix(int a, int b) {
    volatile int local_vol = 0;
    
    /* if (a != b) { modify a, use volatile } */
    if (a != b) {
        a = a ^ b;                   /* Modifies 'a' from test expression */
        local_vol = a;               /* Volatile write */
        b = b + local_vol;           /* Read volatile, modify 'b' */
        sink = a * b;
        
        /* More statements */
        int temp = a << 2;
        dummy_op(&temp);
        a = temp;                    /* Another modification of 'a' */
    }
    sink = a + b + local_vol;
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize some test data */
    int array[5] = {100, 200, 300, 400, 500};
    int *ptr = &array[2];
    
    printf("Starting ifcvt test...\n");
    
    /* Test 1: Single modification */
    test_single_modification(glob_a, glob_b);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(15, 10, 25, 30);
    result += sink;
    
    /* Test 3: Indirect modification */
    test_indirect_modification(ptr, 250);
    result += sink;
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    result += sink + glob_c;
    
    /* Test 5: Complex condition */
    test_complex_condition(50, 30, 5);
    result += sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix(7, 13);
    result += sink;
    
    /* Additional test with global variables in condition */
    if (glob_a > glob_b && glob_c < glob_d) {
        glob_a = glob_a + glob_c;    /* Modifies glob_a from condition */
        glob_b = glob_b * 2;         /* Modifies glob_b from condition */
        sink = glob_a ^ glob_b;
        result += sink;
    }
    
    printf("Result checksum: %d\n", result);
    printf("Global state: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
