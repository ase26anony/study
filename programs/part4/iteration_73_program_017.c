/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return a > b;
}

static void __attribute__((noinline, noipa)) dummy_use(int val) {
    sink = val;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int x, int y) {
    /* Test expression uses x and y */
    if (x > y && glob_c != 0) {
        /* Modify x which appears in test expression */
        x = x + 1;
        /* Additional non-debug instructions */
        y = y * 2;
        glob_a = glob_a ^ 0x55;
        dummy_use(x + y);
    }
    sink = x;
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound test expression */
    if ((a > b) || (c < d)) {
        /* Modify both a and c from test expression */
        a = a + glob_b;
        /* Additional arithmetic to ensure real instructions */
        b = b | 0xF0;
        c = c * 3;
        d = d / 2;
        /* Use volatile to prevent elimination */
        sink = a + b + c + d;
    }
    dummy_use(a);
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Test expression uses pointer values */
    if (ptr1 != NULL && *ptr1 > *ptr2) {
        /* Modify through pointer - affects *ptr1 used in condition */
        *ptr1 = *ptr1 + 100;
        /* Additional operations */
        *ptr2 = *ptr2 - 50;
        int temp = *ptr1 ^ *ptr2;
        dummy_use(temp);
    }
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int counter = get_value();
    int threshold = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Test expression uses counter */
        if (counter > threshold && glob_a < glob_b) {
            /* Modify counter which is in test expression */
            counter = counter / 2;
            /* Additional statements to create basic block */
            threshold = threshold + 1;
            glob_c = glob_c ^ counter;
            dummy_use(counter);
        } else {
            counter = counter * 3 + 1;
        }
    }
    sink = counter;
}

/* Test 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y, int z) {
    /* Complex test expression */
    if (check_cond(x, y) || (z != 0 && glob_a > 0)) {
        /* Modify x which appears in check_cond argument */
        x = x * x + y;
        /* Modify glob_a which appears in test expression */
        glob_a = glob_a - 1;
        /* Multiple additional operations */
        y = y << 2;
        z = z | 0xAA;
        modify(&glob_b);
        dummy_use(x + y + z);
    }
}

/* Test 6: Modification in both then and else with external side effects */
static void __attribute__((noinline, noipa)) test_both_branches_modify(int a, int b) {
    volatile int local_sink = 0;
    
    if (a > b) {
        /* Modify a which is in test expression */
        a = a + b;
        /* Additional operations to create substantial basic block */
        b = b * a;
        local_sink = a ^ b;
        glob_c = glob_c + 1;
    } else {
        /* Also modify b in else branch */
        b = b - a;
        a = a | 0xFF;
        local_sink = a & b;
        glob_c = glob_c - 1;
    }
    sink = local_sink;
}

/* Test 7: Pointer derived from test variable */
static void __attribute__((noinline, noipa)) test_pointer_derived(int base) {
    int array[10];
    int *ptr = array + (base % 10);
    
    /* Test uses ptr */
    if (ptr != NULL && base > 0) {
        /* Modification through derived pointer */
        *ptr = 42;
        /* Additional modifications */
        base = base + *ptr;
        array[0] = base;
        dummy_use(*ptr);
    }
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-constant values */
    int arg1 = get_value();
    int arg2 = get_value();
    int arg3 = get_value();
    int arg4 = get_value();
    
    /* Test 1: Single modification */
    test_single_modification(arg1, arg2);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(arg1, arg2, arg3, arg4);
    result += sink;
    
    /* Test 3: Indirect modification */
    int val1 = 100, val2 = 200;
    test_indirect_modification(&val1, &val2);
    result += val1 + val2;
    
    /* Test 4: Loop nested */
    test_loop_nested(10);
    result += sink;
    
    /* Test 5: Complex condition */
    test_complex_condition(arg1, arg2, arg3);
    result += sink;
    
    /* Test 6: Both branches modify */
    test_both_branches_modify(arg1, arg2);
    result += sink;
    
    /* Test 7: Pointer derived */
    test_pointer_derived(arg1);
    result += sink;
    
    /* Final output to prevent complete optimization */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
