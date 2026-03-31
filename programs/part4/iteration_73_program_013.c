/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targeting lines 577-583: modified_in_p detection */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink; /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink = val;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return a > b;
}

static int __attribute__((noinline, noipa)) modify_value(int x) {
    return x * 2 + 1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Test expression uses 'a' */
    if (a > b) {
        /* Modify the test expression variable 'a' */
        a = a + 1;           /* Direct modification */
        a = a * 3;           /* Another modification */
        int temp = a & 0xFF; /* Bitwise operation using 'a' */
        use_value(temp);
        /* Additional statement to flesh out basic block */
        sink = a + b;
    }
    use_value(a); /* Ensure 'a' is used after modification */
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound test expression */
    if ((a > b) || (c < d)) {
        /* Modify both variables from different parts of the condition */
        a = a + b;      /* Modifies 'a' from first part */
        c = c * 2;      /* Modifies 'c' from second part */
        b = b ^ 0x55;   /* Additional modification */
        /* Multiple statements to ensure loop iteration */
        int t1 = a * c;
        int t2 = b + d;
        use_value(t1 + t2);
    }
    sink = a + b + c + d;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Test expression uses the pointer value */
    if (ptr != NULL && *ptr > threshold) {
        /* Indirect modification of what the pointer points to */
        *ptr = *ptr + 42;      /* Modification through dereference */
        int temp = *ptr >> 2;  /* Additional computation */
        ptr[0] = temp;         /* Another write */
        use_value(*ptr);
    }
    if (ptr) {
        sink = *ptr;
    }
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression with loop-varying variables */
        if (a > b && (i % 3) != 0) {
            /* Modify test expression variables */
            a = a + i;          /* Loop-carried modification */
            b = b - 1;          /* Another modification */
            /* Multiple statements in then block */
            int prod = a * b;
            int sum = a + b;
            use_value(prod - sum);
        } else {
            /* Else block to preserve control flow */
            a = a ^ i;
        }
        /* Loop side effect to prevent elimination */
        sink = a + b + i;
    }
    glob_a = a; /* Store back to global */
    glob_b = b;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y, int z) {
    /* Complex test expression */
    if (cond_check(x, y) && (z != 0) && (x + y < 100)) {
        /* Modify multiple variables from the condition */
        x = modify_value(x);   /* x appears in cond_check and x+y<100 */
        y = y >> 1;            /* y appears in cond_check and x+y<100 */
        z = z * z;             /* z appears in z != 0 */
        
        /* Sequence of statements */
        int r1 = x | y;
        int r2 = z & 0xFF;
        use_value(r1 + r2);
        
        /* Another modification of test expression variable */
        x = x ^ y;
    }
    sink = x + y + z;
}

/* Test case 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int v1 = get_value();
    volatile int v2 = get_value();
    int local = v1;
    
    if (local > v2 && v1 != 0) {
        /* Modify variable used in condition */
        local = local + v2;     /* local appears in local > v2 */
        v1 = v1 * 2;            /* v1 appears in v1 != 0 (through volatile) */
        
        /* Multiple arithmetic operations */
        int t = local * 3;
        t = t / 2;
        t = t | 0xAA;
        use_value(t);
        
        /* Another modification */
        local = local ^ 0x55;
    }
    sink = local + v1;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int a = get_value();
    int b = get_value();
    int c = get_value();
    int d = get_value();
    
    /* Array for pointer test */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_value();
    }
    
    /* Run all test cases */
    test_single_modification(a, b);
    result += sink;
    
    test_multiple_modifications(a, b, c, d);
    result += sink;
    
    test_indirect_modification(&arr[3], 50);
    result += sink;
    
    test_loop_nested(10);
    result += glob_a + glob_b;
    
    test_complex_condition(a, b, c);
    result += sink;
    
    test_volatile_mix();
    result += sink;
    
    /* Print checksum to ensure execution */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
