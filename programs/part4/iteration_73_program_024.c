/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink;  /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    *ptr = (*ptr * 3) + 7;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent simplification */
    if (a > b && (a % 7) != 0) {
        /* Modify test expression variable 'a' */
        a = a * 2 + 1;
        /* Additional non-debug instructions */
        b = b ^ 0xFF;
        sink = a + b;  /* Prevent elimination */
        /* Call opaque function */
        modify(&a);
    }
    sink = a;  /* Use result */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int x, int y, int z) {
    /* Compound conditional using multiple variables */
    if ((x > y) || (z != 0 && (x + y) < 100)) {
        /* Modify multiple test expression variables */
        x = x + y;      /* Modifies 'x' used in condition */
        y = y * 3;      /* Modifies 'y' used in condition */
        z = z | 0x01;   /* Modifies 'z' used in condition */
        
        /* Additional arithmetic to create more instructions */
        int temp = x * y;
        sink = temp + z;
        
        /* Another modification */
        x = get_value(x);
    }
    sink = x + y + z;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition tests pointer and value */
    if (ptr != NULL && *ptr < threshold) {
        /* Indirect modification of value tested in condition */
        *ptr = *ptr * 2 + 5;
        
        /* Additional operations */
        int temp = *ptr ^ 0x12345678;
        sink = temp;
        
        /* Another indirect modification */
        modify(ptr);
    }
    if (ptr) sink = *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int n, int *arr) {
    int sum = 0;
    volatile int loop_sink = 0;
    
    for (int i = 0; i < n; i++) {
        /* Condition uses loop variable and array element */
        if (i > 0 && arr[i] > arr[i-1]) {
            /* Modify array element used in condition */
            arr[i] = arr[i] + i;
            
            /* Additional operations in then block */
            sum = sum ^ arr[i];
            loop_sink = sum;
            
            /* Modify loop variable indirectly */
            arr[i-1] = get_value(arr[i-1]);
        }
        
        /* Prevent optimization */
        loop_sink = arr[i % n];
    }
    sink = sum;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Condition with function call */
    if (check_condition(a, b) && c != 0) {
        /* Modify variables used in condition */
        a = a + b + c;
        c = c >> 1;
        
        /* Multiple statements in then block */
        int t1 = a * b;
        int t2 = c ^ 0xAA;
        sink = t1 + t2;
        
        /* Another modification */
        b = get_value(b);
    }
    sink = a + b + c;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Condition uses volatile globals */
    if (local_a > local_b && glob_c > 0) {
        /* Modify local variables derived from volatiles */
        local_a = local_a * glob_c;
        local_b = local_b + 1;
        
        /* Access volatile in then block */
        sink = glob_a;
        
        /* More operations */
        local_a = local_a ^ local_b;
        sink = local_a;
    }
    sink = local_a + local_b;
}

/* Test 7: Bitwise operations in condition and modification */
static void __attribute__((noinline, noipa)) test_bitwise_ops(int x, int y) {
    /* Bitwise condition */
    if ((x & 0xF) == (y & 0xF) || (x | y) > 100) {
        /* Modify variables with bitwise ops */
        x = x ^ y;      /* Modifies x used in condition */
        y = y << 2;     /* Modifies y used in condition */
        
        /* Multiple statements */
        int z = x & y;
        sink = z;
        
        x = x | 0x80000000;
        y = y & 0x7FFFFFFF;
    }
    sink = x - y;
}

/* Main function that exercises all tests */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Run test cases with various inputs */
    test_single_modification(glob_a, glob_b);
    result += sink;
    
    test_multiple_modifications(25, 15, 5);
    result += sink;
    
    int test_val = 42;
    test_indirect_modification(&test_val, 100);
    result += sink;
    
    test_loop_nested(8, data);
    result += sink;
    
    test_complex_condition(100, 50, 25);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    test_bitwise_ops(0x12345678, 0x87654321);
    result += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    
    /* Additional volatile writes to prevent optimization */
    glob_a = result;
    glob_b = result ^ 0x55AA55AA;
    
    return 0;
}
