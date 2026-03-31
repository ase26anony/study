/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink ^= val;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((rand() & 1) == 0);
}

static int __attribute__((noinline, noipa)) modify_var(int x) {
    return x * 2 + 1;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b) {
        /* Modify the test expression variable 'a' */
        a = a + 1;
        /* Additional non-debug instructions */
        int temp = a * 3;
        b = b - temp;
        /* Use volatile to prevent elimination */
        sink = a + b;
    }
    use_value(a + b);
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound test expression */
    if ((a > b) && (c != d)) {
        /* Modify both variables from test expression */
        a = modify_var(a);
        c = c * 2;
        /* Additional arithmetic */
        b = b ^ 0x55;
        d = d + a;
        /* Multiple real instructions */
        sink = a + b + c + d;
    }
    use_value(a + b + c + d);
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses value pointed by ptr */
    if (ptr && (*ptr > threshold)) {
        /* Modify through pointer - affects test expression */
        *ptr = *ptr / 2;
        /* Additional operations */
        int temp = *ptr + 100;
        sink = temp;
        /* Another modification */
        *ptr = *ptr ^ 0xAA;
    }
    if (ptr) use_value(*ptr);
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = get_value();
    int y = get_value();
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression with loop variables */
        if (x < y && (i % 3) != 0) {
            /* Modify test expression variable */
            x = x + i;
            /* Additional statements */
            y = y - (x >> 1);
            int z = x * y;
            sink ^= z;
        }
        /* Loop-carried dependency */
        x = x ^ (y + i);
        y = y + (i & 0xF);
    }
    use_value(x + y);
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b, int c) {
    /* Complex test expression */
    if (check_cond(a, b) || (c != 0 && a < 100)) {
        /* Modify variable from test expression */
        a = a | 0x1;
        /* Multiple modifications */
        b = b & ~0x1;
        c = c + a;
        /* Sequence of real instructions */
        int t1 = a * b;
        int t2 = c ^ t1;
        sink = t2;
        /* Another modification of test variable */
        a = a + t2;
    }
    use_value(a + b + c);
}

/* Test 6: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_condition(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Test expression uses volatiles */
    if (local_a > local_b && glob_c != 0) {
        /* Modify variables used in condition */
        local_a = local_a * 2;
        glob_c = glob_c - 1;  /* Affects glob_c in condition */
        /* Additional operations */
        local_b = local_b + local_a;
        sink = local_a + local_b + glob_c;
    }
    use_value(local_a + local_b);
}

/* Test 7: Multiple basic blocks with modifications */
static void __attribute__((noinline, noipa)) test_multi_block(int a, int b) {
    if (a > 0) {
        /* First modification in then block */
        a = a - 1;
        b = b + a;
        
        /* Nested condition */
        if (b < 100) {
            /* Modify again */
            a = a ^ b;
            sink = a;
        }
        
        /* More modifications */
        a = a * 2;
        b = b / 2;
    }
    use_value(a + b);
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int val1 = get_value();
    int val2 = get_value();
    int val3 = get_value();
    int val4 = get_value();
    
    /* Dynamic allocation for pointer test */
    int *dynamic_ptr = malloc(sizeof(int));
    if (dynamic_ptr) {
        *dynamic_ptr = get_value();
    }
    
    /* Run all test cases */
    test_single_modification(val1, val2);
    test_multiple_modifications(val1, val2, val3, val4);
    
    if (dynamic_ptr) {
        test_indirect_modification(dynamic_ptr, 50);
    }
    
    test_loop_nested(10);
    test_complex_condition(val1, val2, val3);
    test_volatile_condition();
    test_multi_block(val1, val2);
    
    /* Clean up */
    if (dynamic_ptr) {
        free(dynamic_ptr);
    }
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", sink);
    
    return 0;
}
