/* Test program for if-conversion modification analysis */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink; /* Prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a * b) > 100;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Complex condition to prevent simplification */
    if (a > b && glob_c != 0) {
        /* Modify variable used in condition */
        a = a + glob_a;  /* This modifies 'a' from test expression */
        
        /* Additional non-debug instructions */
        glob_b = glob_b ^ 0x1234;
        sink = a * 2;
        
        /* Another modification */
        a = get_value(a);
    }
    sink = a + b; /* Use result */
}

/* Test 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multiple_modifications(int a, int b, int c, int d) {
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || (glob_a != 0)) {
        /* Modify multiple variables from the condition */
        a = a * 3 + 1;      /* Modifies 'a' */
        c = c ^ d;          /* Modifies 'c' */
        glob_a = glob_a - 5; /* Modifies global used in condition */
        
        /* Additional arithmetic to flesh out basic block */
        b = (b << 2) | 1;
        d = d % 17;
        
        /* Function call that might affect variables */
        modify(&glob_b);
    }
    sink = a + b + c + d;
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr1, int *ptr2) {
    /* Condition based on pointer values */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        /* Indirect modification of value used in condition */
        *ptr1 = *ptr1 + 42;  /* Modifies *ptr1 which was in condition */
        
        /* Additional operations */
        *ptr2 = (*ptr2) * 2;
        glob_c = glob_c ^ *ptr1;
        
        /* More complex modification */
        int temp = *ptr1;
        *ptr1 = temp >> 1;
    }
    sink = ptr1 ? *ptr1 : 0;
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int x = glob_a;
    int y = glob_b;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Condition using variables modified in loop */
        if (x > y && check_cond(x, y)) {
            /* Modify variables used in condition */
            x = x + i;      /* Modifies 'x' from condition */
            y = y - glob_d; /* Modifies 'y' from condition */
            
            /* Additional statements in basic block */
            glob_c = glob_c ^ (x & 0xFF);
            sink = x * y;
            
            /* Another modification */
            x = (x << 3) | (x >> 29); /* Rotate */
        }
        
        /* Loop-carried dependency to prevent optimization */
        y = y + get_value(i);
    }
    sink = x + y;
}

/* Test 5: Complex condition with function call and modification */
static void __attribute__((noinline, noipa)) test_complex_condition(int a, int b) {
    /* Complex condition that can't be evaluated at compile time */
    if ((a * b > 100) || (glob_a < glob_b) || check_cond(a, glob_c)) {
        /* Modify variable used in first part of condition */
        a = a ^ b;  /* Modifies 'a' used in 'a * b > 100' */
        
        /* Multiple statements in then block */
        b = b + glob_d;
        glob_a = glob_a | 0x80000000;
        
        /* Function call with side effect */
        modify(&glob_b);
        
        /* Another arithmetic operation */
        a = a % 31;
    }
    sink = a * 100 + b;
}

/* Test 6: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_condition(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    if (local_a > local_b && glob_c != 0) {
        /* Modify volatile variable used in condition */
        local_a = local_a + glob_d;  /* This should be preserved */
        
        /* Multiple operations */
        local_b = local_b ^ 0xDEADBEEF;
        glob_c = glob_c * 2;
        
        /* Additional computation */
        int temp = local_a * local_b;
        sink = temp;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize some local variables */
    int var1 = 100;
    int var2 = 200;
    int var3 = 300;
    int var4 = 400;
    
    int arr1[2] = {50, 60};
    int arr2[2] = {70, 80};
    
    printf("Starting if-conversion modification tests...\n");
    
    /* Run test 1: Single modification */
    test_single_modification(var1, var2);
    result += sink;
    
    /* Run test 2: Multiple modifications */
    test_multiple_modifications(var1, var2, var3, var4);
    result += sink;
    
    /* Run test 3: Indirect modification */
    test_indirect_modification(&arr1[0], &arr2[0]);
    result += sink;
    
    /* Run test 4: Loop-nested */
    test_loop_nested(5);
    result += sink;
    
    /* Run test 5: Complex condition */
    test_complex_condition(var1, var3);
    result += sink;
    
    /* Run test 6: Volatile condition */
    test_volatile_condition();
    result += sink;
    
    /* Final result to prevent optimization */
    printf("Test result checksum: %d\n", result);
    
    /* Verify modifications */
    printf("Global values: a=%d, b=%d, c=%d, d=%d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
