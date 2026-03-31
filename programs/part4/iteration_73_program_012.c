/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Lines 577-583: modified_in_p check in then_bb */

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

static void __attribute__((noinline, noipa)) modify_a(int *a) {
    *a = (*a * 3) / 2 + 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

static void __attribute__((noinline, noipa)) noop_sink(int val) {
    sink += val;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) modifies a in then block */
    if (a > b) {
        /* Multiple statements including modification of test expression variable */
        a = a + 1;           /* Direct modification of test variable */
        int temp = b * 2;    /* Additional computation */
        a = a ^ temp;        /* Another modification of test variable */
        noop_sink(a + b);    /* Prevent dead code elimination */
    }
    sink += a;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modification(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) && (c != d)) {
        /* Modify both variables from the test expression */
        a = a * 2 + 1;      /* Modify first test variable */
        c = c ^ d;          /* Modify second test variable */
        b = b + a;          /* Additional modification */
        noop_sink(a + b + c + d);
    }
    sink += a + c;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr_a, int *ptr_b) {
    /* Test expression uses dereferenced pointers */
    if (ptr_a && ptr_b && (*ptr_a > *ptr_b)) {
        /* Modify through pointer - affects test expression */
        *ptr_a = *ptr_a / 2;
        *ptr_b = *ptr_b + 10;
        int temp = *ptr_a * *ptr_b;
        noop_sink(temp);
    }
    if (ptr_a) sink += *ptr_a;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x > y && (x % 3) == 0) {
            /* Modify test expression variables */
            x = x + y + i;
            y = y * 2 - i;
            /* Additional non-debug instructions */
            int z = x ^ y;
            noop_sink(z);
        } else {
            x = x - 1;
            y = y + 1;
        }
    }
    sink += x + y;
}

/* Test 5: Complex arithmetic in test expression with modification */
static void __attribute__((noinline, noipa)) test_complex_expr(int a, int b, int c) {
    /* Complex test expression */
    if ((a * b > c) && ((a ^ b) < (c + 10))) {
        /* Modify multiple variables from test expression */
        a = (a << 2) | 1;
        b = b ^ 0x12345678;
        c = c * 3 + a;
        
        /* Sequence of statements to ensure basic block has real insns */
        int t1 = a + b;
        int t2 = c - b;
        a = t1 ^ t2;  /* Another modification of test variable */
        noop_sink(t1 + t2);
    }
    sink += a + b + c;
}

/* Test 6: Function call in condition with modification */
static void __attribute__((noinline, noipa)) test_func_cond(int a, int b) {
    /* Condition uses function call */
    if (check_cond(a, b)) {
        /* Modify variables used in condition */
        a = get_value(a);
        b = b + glob_c;
        modify_a(&a);  /* Another modification through function */
        noop_sink(a * b);
    }
    sink += a - b;
}

/* Test 7: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    /* Mix volatile and non-volatile in condition */
    if (local_a > glob_b && glob_c != local_b) {
        /* Modify both local and global (volatile) variables */
        local_a = local_a * glob_c;
        glob_b = glob_b + 1;  /* This affects the volatile in condition! */
        local_b = local_b ^ local_a;
        noop_sink(local_a + local_b);
    }
    sink += local_a;
}

int main(void) {
    int result = 0;
    
    /* Initialize some heap memory for pointer test */
    int *ptr1 = malloc(sizeof(int));
    int *ptr2 = malloc(sizeof(int));
    if (ptr1 && ptr2) {
        *ptr1 = 100;
        *ptr2 = 50;
    }
    
    /* Run all test cases with varying inputs */
    printf("Running if-conversion test cases...\n");
    
    /* Test 1: Single modification */
    test_single_modification(glob_a, glob_b);
    printf("Test 1 completed\n");
    
    /* Test 2: Multiple modifications */
    test_multiple_modification(15, 10, 25, 30);
    printf("Test 2 completed\n");
    
    /* Test 3: Indirect modification */
    if (ptr1 && ptr2) {
        test_indirect_modification(ptr1, ptr2);
    }
    printf("Test 3 completed\n");
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    printf("Test 4 completed\n");
    
    /* Test 5: Complex expression */
    test_complex_expr(12, 8, 100);
    printf("Test 5 completed\n");
    
    /* Test 6: Function in condition */
    test_func_cond(20, 15);
    printf("Test 6 completed\n");
    
    /* Test 7: Volatile mix */
    test_volatile_mix();
    printf("Test 7 completed\n");
    
    /* Cleanup */
    free(ptr1);
    free(ptr2);
    
    /* Final checksum */
    result = sink;
    printf("Final checksum: %d\n", result);
    printf("All tests completed successfully.\n");
    
    return 0;
}
