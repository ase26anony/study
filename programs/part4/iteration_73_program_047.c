/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink;  /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    *ptr = (*ptr * 3) / 2;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return (a > b) && ((a ^ b) & 1);
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) { a = a + 1; ... } */
    if (a > b) {
        /* Multiple non-debug instructions */
        a = a + 1;           /* Modifies test expression variable */
        sink = a * b;        /* Prevent optimization */
        a = a ^ 0x1234;      /* Another modification */
        sink = a + b;
    }
    sink = a;  /* Use result */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* if (a > b || c < d) { modify both a and c } */
    if (a > b || c < d) {
        /* Modify variables used in the condition */
        a = get_value(a);    /* Modifies 'a' from first part of condition */
        c = c * 2 + 1;       /* Modifies 'c' from second part of condition */
        b = b ^ c;           /* Additional computation */
        sink = a + b + c + d;
    }
    sink = a + c;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* if (ptr && *ptr > threshold) { *ptr = 42; } */
    if (ptr && *ptr > threshold) {
        *ptr = 42;           /* Indirect modification through ptr */
        int temp = *ptr * 2;
        sink = temp;
        *ptr = *ptr + 1;     /* Another indirect modification */
    }
    if (ptr) sink = *ptr;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex condition using loop-varying variables */
        if (a > b && (a ^ i) != 0) {
            /* Modify variables used in condition */
            a = a + i;          /* Modifies 'a' from condition */
            b = b ^ a;          /* Modifies 'b' from condition */
            sink = a * b + i;   /* Side effect */
            
            /* Additional non-debug instructions */
            int temp = a << 2;
            b = b + temp;
        }
        /* Loop-carried dependency */
        a = a ^ 0x1;
        b = b + 1;
    }
    sink = a + b;
}

/* Test 5: Function call that modifies condition variable */
static void __attribute__((noinline, noipa)) test_function_modification(int a, int b) {
    /* if (check_condition(a, b)) { modify(&a); ... } */
    if (check_condition(a, b)) {
        modify(&a);            /* Function modifies 'a' used in condition */
        a = a | 0x100;         /* Direct modification */
        b = b & 0xFF;
        sink = a * b;
        
        /* More instructions to flesh out basic block */
        int temp = a + b;
        a = temp ^ a;
        sink = a;
    }
    sink = b;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    int local = glob_c;
    
    /* Condition uses volatile and non-volatile */
    if (v1 > local && v2 != 0) {
        /* Modify the non-volatile variable used in condition */
        local = local * 3 + 1;
        v1 = v1 ^ local;      /* Also modify volatile */
        sink = local + v1 + v2;
        
        /* Additional arithmetic */
        local = (local << 1) | 1;
        sink = local;
    }
    sink = v1 + local;
}

/* Test 7: Bitwise operations in condition and modification */
static void __attribute__((noinline, noipa)) test_bitwise_ops(int x, int y, int mask) {
    /* if ((x & mask) && (y | mask) != mask) { modify x and y } */
    if ((x & mask) && (y | mask) != mask) {
        x = x ^ y;            /* Modifies x used in condition */
        y = y & ~mask;        /* Modifies y used in condition */
        int temp = x | y;
        x = temp ^ mask;
        sink = x + y;
        
        /* More operations */
        y = y << 2;
        x = x >> 1;
        sink = x * y;
    }
    sink = x ^ y;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int a = get_value(100);
    int b = get_value(200);
    int c = get_value(300);
    int d = get_value(400);
    
    /* Test 1: Single modification */
    test_single_modification(a, b);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(a, b, c, d);
    result += sink;
    
    /* Test 3: Indirect modification */
    int value = 50;
    test_indirect_modification(&value, 25);
    result += sink;
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    result += sink;
    
    /* Test 5: Function modification */
    test_function_modification(a, b);
    result += sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix();
    result += sink;
    
    /* Test 7: Bitwise operations */
    test_bitwise_ops(a, b, 0xFF);
    result += sink;
    
    /* Additional complex case: combination in main */
    int x = glob_a;
    int y = glob_b;
    int z = glob_c;
    
    for (int i = 0; i < 3; i++) {
        if ((x > y && z != 0) || (x ^ y) < z) {
            /* Modify all three variables used in condition */
            x = x + i + 1;
            y = y ^ x;
            z = z * 2 - 1;
            
            /* Additional statements to create more instructions */
            int tmp = x * y;
            sink = tmp + z;
            x = x | 0x1;
            y = y & 0xFFFFFFFE;
        }
        /* Loop update with dependency */
        x = x + z;
        y = y - i;
        z = z ^ 0x55AA;
    }
    result += x + y + z + sink;
    
    printf("Result checksum: %d\n", result & 0xFF);
    return 0;
}
