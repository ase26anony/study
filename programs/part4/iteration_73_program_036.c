/* Test program for if-conversion uncovered lines in ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink for results to prevent elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink += val;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

static int __attribute__((noinline, noipa)) modify(int x) {
    return x * 3 + 1;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* if (a > b) { a = modify(a); ... } */
    if (a > b) {
        /* Modify the test expression variable 'a' */
        a = modify(a);
        
        /* Additional non-debug instructions */
        int temp = a * 2;
        use_value(temp);
        
        /* Another modification to ensure multiple instructions */
        a = a ^ 0x55;
        use_value(a);
    }
    
    /* Use result to prevent dead code elimination */
    sink += a + b;
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    int d = glob_d;
    
    /* if ((a > b) && (c != d)) { modify both a and c } */
    if ((a > b) && (c != d)) {
        /* Modify both variables used in the test expression */
        a = a + 1;
        c = c * 2;
        
        /* Additional computation to flesh out the basic block */
        int sum = a + b + c + d;
        use_value(sum);
        
        /* Another modification */
        a = a | 0x0F;
        c = c & 0xF0;
        
        /* Use volatile sink */
        sink += a * c;
    }
    
    /* Consume results */
    sink += a + c;
}

/* Test 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int x = glob_a;
    int y = glob_b;
    int *ptr = &x;
    
    /* if (x > y) { modify through pointer derived from x } */
    if (x > y) {
        /* Indirect modification - ptr is related to test variable x */
        *ptr = 42;
        
        /* Additional operations */
        int temp = *ptr * 3;
        use_value(temp);
        
        /* Direct modification as well */
        x = x + 100;
        
        /* More operations to ensure multiple instructions */
        y = y ^ x;
        use_value(y);
    }
    
    sink += x + y;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int iterations = 5;
    
    for (int i = 0; i < iterations; i++) {
        /* if (a < b) { modify a, creating loop-carried dependency } */
        if (a < b) {
            /* Modify test expression variable 'a' */
            a = a + i;
            
            /* Additional statements */
            int product = a * b;
            use_value(product);
            
            /* Another modification */
            a = a ^ (i * 2);
            
            /* Use result */
            sink += a;
        } else {
            /* Else branch to preserve control flow */
            b = b - 1;
        }
        
        /* Loop-carried dependency */
        a = a + 1;
    }
    
    sink += a * b;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    
    /* if (cond_check(a, b) || c > 0) { modify a and c } */
    if (cond_check(a, b) || c > 0) {
        /* Modify variables from test expression */
        a = modify(a);
        c = c << 1;
        
        /* Multiple instructions in the block */
        int diff = a - b;
        use_value(diff);
        
        a = a % 100;
        c = c | 0xAA;
        
        /* More operations */
        int result = a * c;
        use_value(result);
    }
    
    sink += a + b + c;
}

/* Test 6: Modification through array indexing */
static void __attribute__((noinline, noipa)) test_array_mod(void) {
    int arr[4] = {glob_a, glob_b, glob_c, glob_d};
    int idx = glob_a % 4;
    
    /* if (arr[idx] > 0) { modify arr[idx] } */
    if (arr[idx] > 0) {
        /* Modify the array element used in the test */
        arr[idx] = arr[idx] * 2 + 1;
        
        /* Additional operations */
        int sum = 0;
        for (int i = 0; i < 4; i++) {
            sum += arr[i];
        }
        use_value(sum);
        
        /* Another modification */
        arr[idx] = arr[idx] ^ 0xFF;
        
        /* Use result */
        sink += arr[idx];
    }
    
    /* Consume array */
    for (int i = 0; i < 4; i++) {
        sink += arr[i];
    }
}

/* Test 7: Mixed operations with volatile */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int v = glob_a;
    int a = glob_b;
    int b = glob_c;
    
    /* if (a > v) { modify a and v } */
    if (a > v) {
        /* Modify test expression variables */
        a = a + v;
        v = v * 2;
        
        /* Mixed operations */
        int temp = a & b;
        use_value(temp);
        
        a = a >> 1;
        v = v + 1;
        
        /* More operations */
        sink += a * v;
    }
    
    sink += a + v;
}

int main(void) {
    /* Initialize random seed for non-deterministic behavior */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    test_multi_var_mod();
    test_indirect_mod();
    test_loop_nested();
    test_complex_cond();
    test_array_mod();
    test_volatile_mix();
    
    /* Print checksum to verify execution */
    printf("Checksum: %d\n", sink);
    
    return 0;
}
