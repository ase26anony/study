/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink; /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = (*x * 3) + 7;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b && a != 0) {
        /* Modify 'a' which appears in the test expression */
        a = a + 1;                    /* Direct modification */
        a = a * 2;                    /* Another modification */
        sink = a;                     /* Use result to prevent elimination */
        /* Additional non-debug instruction */
        glob_c = glob_c ^ a;
    }
    sink = a + b; /* Ensure value is used */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d)) {
        /* Modify both 'a' and 'c' which appear in the test expression */
        a = a * 3 + 5;                /* Modify first test variable */
        c = c / 2;                    /* Modify second test variable */
        b = b ^ 0xFF;                 /* Additional modification */
        sink = a + c + b;             /* Use results */
        
        /* Call opaque function that might affect variables */
        modify(&a);
    }
    /* Use all variables to prevent dead code elimination */
    sink = a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses the value pointed by ptr */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify the value through pointer - affects test expression */
        *ptr = *ptr + 100;            /* First modification */
        *ptr = (*ptr * 2) & 0xFFFF;   /* Second modification */
        sink = *ptr;                  /* Use result */
        
        /* Additional arithmetic */
        int temp = *ptr ^ 0x1234;
        glob_d = glob_d + temp;
    }
    if (ptr) sink = *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses variables modified in loop */
        if (x < y && x > 0) {
            /* Modify 'x' which is in the test expression */
            x = x + i;                /* Loop-dependent modification */
            x = x | 0x1;              /* Additional modification */
            y = y - 1;                /* Modify other test variable */
            sink = x ^ y;             /* Use results */
            
            /* Call opaque function */
            x = get_value(x);
        }
        /* Ensure loop has side effect */
        glob_c = glob_c + x + y;
    }
    sink = x + y;
}

/* Test 5: Complex test with function call in condition */
static void __attribute__((noinline, noipa)) test_function_in_condition(int a, int b) {
    /* Opaque function call in condition */
    if (check_cond(a, b) && a > 10) {
        /* Modify 'a' which appears in the test expression */
        a = a * a;                    /* Square operation */
        a = a % 1000;                 /* Additional modification */
        b = b + a;                    /* Modify other variable */
        sink = a - b;                 /* Use results */
        
        /* Multiple statements to ensure basic block has content */
        volatile int local_sink = 0;
        local_sink = a ^ b;
        glob_a = local_sink;
    }
    sink = a * b;
}

/* Test 6: Volatile access in test expression modification */
static void __attribute__((noinline, noipa)) test_volatile_interaction(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    
    if (v1 > v2 && glob_c != 0) {
        /* Modify volatile variable used in condition */
        v1 = v1 * 2;                  /* This affects future reads of v1 */
        glob_c = glob_c - 1;          /* Modify other test variable */
        sink = v1 + v2 + glob_c;      /* Use results */
        
        /* Additional non-trivial computation */
        int temp = (v1 << 3) | (v2 & 0xF);
        modify(&temp);
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value(100);
    int arg2 = get_value(200);
    int arg3 = get_value(300);
    int arg4 = get_value(400);
    
    /* Test 1: Single modification */
    test_single_modification(arg1, arg2);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    /* Test 3: Indirect modification */
    int data = 500;
    int *ptr = &data;
    test_indirect_modification(ptr, 400);
    result += sink + data;
    
    /* Test 4: Loop-nested */
    test_loop_nested(5);
    result += sink;
    
    /* Test 5: Function in condition */
    test_function_in_condition(arg1, arg2);
    result += sink;
    
    /* Test 6: Volatile interaction */
    test_volatile_interaction();
    result += sink;
    
    /* Additional test with array access */
    int arr[4] = {arg1, arg2, arg3, arg4};
    if (arr[0] > arr[1] && arr[2] < arr[3]) {
        arr[0] = arr[0] + arr[1];     /* Modify test expression variable */
        arr[2] = arr[2] * 2;          /* Modify another test variable */
        sink = arr[0] ^ arr[2];
        result += sink;
    }
    
    printf("Result checksum: %d\n", result);
    return 0;
}
