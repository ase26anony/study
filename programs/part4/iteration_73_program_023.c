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

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3) + 7;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ 0x1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        glob_c = glob_c ^ 0xFF;
        sink = a * b;
        /* Another modification to ensure multiple instructions */
        a = a * 2;
    }
    sink = a + b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' from test expression */
        a = get_value(a);
        c = c * 3;
        /* Additional arithmetic to flesh out basic block */
        b = b + (a >> 2);
        d = d ^ c;
        sink = a + b + c + d;
        /* Second modification to 'a' */
        a = a & 0x0F;
    }
    sink = a + c;
}

/* Test case 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses value pointed by ptr */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify the value that was tested */
        *ptr = *ptr + 5;
        /* Additional operations */
        int temp = *ptr * 2;
        modify(ptr);  /* Another modification via function call */
        sink = temp + threshold;
        /* One more direct modification */
        *ptr = *ptr % 100;
    }
    if (ptr) sink = *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses loop-varying variables */
        if (a > b && i % 3 != 0) {
            /* Modify 'a' which is in test expression */
            a = a + i;
            /* Additional statements */
            b = b ^ a;
            sink = a * b;
            /* Another modification */
            a = a & 0x7F;
        }
        /* Loop-carried dependency */
        b = b + 1;
    }
    glob_a = a;
    glob_b = b;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Complex test expression */
    if (check_cond(x, y) && (x + y) > 50) {
        /* Modify 'x' which appears in test expression */
        x = x * 3 + y;
        /* Multiple instructions in then block */
        y = y ^ x;
        sink = x - y;
        x = x >> 1;
        /* Call to function that might be analyzed */
        modify(&x);
    }
    sink = x + y;
}

/* Test case 6: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_condition(void) {
    int a = glob_a;
    volatile int v = glob_b;
    
    /* Test expression uses volatile */
    if (a > v && glob_c < glob_d) {
        /* Modify 'a' from test expression */
        a = a + v;
        /* Modify globals from test expression */
        glob_c = glob_c * 2;
        /* Multiple operations */
        v = v ^ a;
        sink = a * glob_c;
        a = a % 100;
    }
    glob_a = a;
}

/* Test case 7: Multiple basic blocks scenario */
static void __attribute__((noinline, noipa)) test_multi_block(int a, int b) {
    /* This creates more complex CFG */
    if (a > 0) {
        if (b > a) {  /* Nested condition */
            /* Modify 'a' used in outer condition */
            a = a * 2 + b;
            /* Additional instructions */
            int c = a ^ b;
            sink = c;
            a = a - b;
        } else {
            /* Else block also modifies test variable */
            a = a / 2;
        }
    }
    sink = a + b;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Allocate memory for pointer test */
    int *ptr = malloc(sizeof(int));
    if (ptr) {
        *ptr = 150;
    }
    
    /* Execute all test cases */
    test_single_modification(arg1, arg2);
    result += sink;
    
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    if (ptr) {
        test_indirect_modification(ptr, 100);
        result += sink;
    }
    
    test_loop_nested(10);
    result += glob_a + glob_b;
    
    test_complex_condition(arg1, arg2);
    result += sink;
    
    test_volatile_condition();
    result += glob_a + glob_c;
    
    test_multi_block(arg3, arg4);
    result += sink;
    
    /* Cleanup */
    if (ptr) {
        free(ptr);
    }
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
