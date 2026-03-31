/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

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
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return a != b;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* if (a > b) modifies a in then block */
    if (a > b) {
        /* Multiple statements including modification of test variable */
        a = a + 1;           /* Direct modification of test expression variable */
        sink = a * b;        /* Use result to prevent elimination */
        a = a ^ 0x1234;      /* Another modification */
        sink = a;            /* Volatile sink */
    }
    sink = a + b;  /* Ensure result is used */
}

/* Test case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound condition using multiple variables */
    if ((a > b) && (c != d)) {
        /* Modify both variables used in condition */
        a = a * 2;           /* Modify first test variable */
        c = c + 1;           /* Modify second test variable */
        b = b ^ a;           /* Additional computation */
        sink = a + b + c + d; /* Use all variables */
        
        /* More statements to ensure basic block has content */
        d = d | 0xFF;
        sink = d;
    }
    sink = a + c;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int val) {
    /* Test expression uses pointer value */
    if (ptr != NULL && *ptr > val) {
        /* Modify through pointer - affects what *ptr evaluates to */
        *ptr = *ptr + 5;     /* Indirect modification */
        int temp = *ptr * 2;
        sink = temp;
        
        /* Additional arithmetic */
        *ptr = *ptr ^ 0xAA;
        sink = *ptr;
    }
    if (ptr) sink = *ptr;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition uses variables modified in loop */
        if (x < y && (i % 3) != 0) {
            /* Modify test expression variable */
            x = x + i;        /* Modification with loop index */
            y = y - 1;        /* Also modify other test variable */
            sink = x * y;     /* Use result */
            
            /* Additional non-trivial computation */
            x = x | 0x1;
            sink = x;
        }
        /* Loop-carried dependency */
        x = x ^ (y << 2);
        sink = x;
    }
    glob_a = x;  /* Store back to global */
}

/* Test case 5: Function call that modifies test expression */
static void __attribute__((noinline, noipa)) test_function_modification(int a, int b) {
    /* Condition based on function result */
    if (check_cond(a, b)) {
        /* Call function that modifies a through pointer */
        modify(&a);          /* Function modifies test variable */
        
        /* Additional statements */
        b = b + a;
        sink = b;
        
        a = a << 3;          /* Another direct modification */
        sink = a;
    }
    sink = a + b;
}

/* Test case 6: Complex expression with volatile reads */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    int a = glob_a;
    int b = glob_b;
    volatile int v = glob_c;
    
    /* Condition with volatile read */
    if (a > (b + v)) {
        /* Modify test variables */
        a = get_value(a);    /* Opaque function call */
        b = b * 2;
        
        /* Multiple arithmetic operations */
        a = a + b;
        b = b ^ a;
        a = a | 0xF0F0;
        
        sink = a;
        sink = b;
        sink = v;
    }
    glob_a = a;
    glob_b = b;
}

/* Test case 7: Pointer derived from test variable */
static void __attribute__((noinline, noipa)) test_derived_pointer(int idx) {
    int array[4] = {10, 20, 30, 40};
    int *ptr = &array[idx % 4];
    
    /* Condition uses pointer value */
    if (ptr != NULL && *ptr > 15) {
        /* Modify through derived pointer */
        *ptr = *ptr * 2;     /* Affects *ptr value */
        int temp = *ptr + 5;
        sink = temp;
        
        /* Modify index variable */
        idx = idx + 1;
        sink = idx;
    }
    sink = array[0] + array[1];
}

int main(void) {
    int result = 0;
    
    printf("Testing if-conversion modification detection...\n");
    
    /* Initialize with non-constant values */
    int a = 100;
    int b = 50;
    int c = 75;
    int d = 75;
    
    /* Test 1: Single modification */
    test_single_modification(a, b);
    result += sink;
    
    /* Test 2: Multiple modifications */
    test_multiple_modifications(a, b, c, d);
    result += sink;
    
    /* Test 3: Indirect modification */
    int val = 60;
    test_indirect_modification(&val, 50);
    result += sink;
    
    /* Test 4: Loop nested */
    test_loop_nested(5);
    result += glob_a + glob_b;
    
    /* Test 5: Function modification */
    test_function_modification(a, b);
    result += sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix();
    result += glob_a + glob_b + glob_c;
    
    /* Test 7: Derived pointer */
    test_derived_pointer(2);
    result += sink;
    
    /* Final checksum */
    printf("Result checksum: %d\n", result);
    printf("All tests completed.\n");
    
    return 0;
}
