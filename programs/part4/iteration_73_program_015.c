/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection */

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
    *x = (*x * 3 + 1) & 0x7FFFFFFF;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return (a > b) ^ (a & 1);
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_modification(int a, int b) {
    /* Condition uses 'a', then block modifies 'a' */
    if (a > b && glob_c != 0) {
        /* Multiple non-debug instructions */
        a = a + 1;                    /* Direct modification of test expression variable */
        glob_a = glob_a ^ a;          /* Additional computation */
        a = a * 3;                    /* Another modification */
        sink = a;                     /* Volatile sink to prevent elimination */
    }
    sink = a + b; /* Ensure value is used */
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_modification(int a, int b, int c, int d) {
    /* Compound condition with multiple variables */
    if ((a > b) || (c < d && glob_a != 0)) {
        /* Modify multiple variables from the condition */
        a = a + glob_b;               /* Modify first test variable */
        c = c * 2 + 1;                /* Modify second test variable */
        b = b ^ c;                    /* Additional computation using modified variable */
        sink = a + b + c;             /* Prevent optimization */
        
        /* Call opaque function to add complexity */
        modify(&a);
    }
    sink = a + b + c + d;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_modification(int *ptr, int threshold) {
    /* Condition uses value pointed by ptr */
    if (ptr != NULL && *ptr > threshold) {
        /* Modify through pointer - affects the value used in condition */
        *ptr = *ptr + 5;              /* Direct modification of dereferenced value */
        int temp = *ptr * 2;          /* Additional computation */
        *ptr = temp / 3;              /* Another modification */
        sink = *ptr;                  /* Volatile sink */
        
        /* Also modify threshold for good measure */
        threshold = threshold + *ptr;
    }
    if (ptr) sink = *ptr + threshold;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(int iterations) {
    int a = glob_a;
    int b = glob_b;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition that changes each iteration */
        if (a > b && i % 3 != 0) {
            /* Modify variables used in condition */
            a = a + i;                /* Modification depends on loop variable */
            b = b - 1;                /* Modify both variables */
            
            /* Multiple statements to create substantial basic block */
            int c = a * b;
            a = a ^ c;
            b = b + (c & 0xFF);
            
            sink = a + b + i;         /* Prevent dead code elimination */
        }
        
        /* Loop-carried dependency */
        a = a + get_value(i);
        b = b - (i & 1);
    }
    
    glob_a = a;
    glob_b = b;
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_condition(int x, int y) {
    /* Opaque function call in condition */
    if (check_cond(x, y) && (x + y) > glob_c) {
        /* Modify variables used in condition */
        x = x * 2 + y;                /* Modify x */
        y = y - glob_d;               /* Modify y */
        
        /* Sequence of operations */
        x = x ^ y;
        y = y | x;
        x = x & ~y;
        
        /* Call function that might modify globals */
        modify(&x);
        
        sink = x - y;
    }
    
    sink = x * y;
}

/* Test case 6: Volatile accesses in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int local_a = glob_a;
    volatile int local_b = glob_b;
    
    /* Condition uses volatile variables */
    if (local_a > local_b || local_a < glob_c) {
        /* Modify volatile variables used in condition */
        local_a = local_a + 1;        /* Direct modification */
        local_b = local_b * 2;        /* Modify both */
        
        /* Additional non-trivial computation */
        int temp = local_a * local_b;
        local_a = local_a ^ temp;
        local_b = local_b | temp;
        
        sink = local_a + local_b;
    }
    
    glob_a = local_a;
    glob_b = local_b;
}

/* Test case 7: Multiple basic blocks within then block */
static void __attribute__((noinline, noipa)) test_multi_bb_simulation(int a, int b) {
    /* This should create a more complex then_bb */
    if (a != b && glob_a > 0) {
        /* First modification */
        a = a + b;
        
        /* Conditional inside then block (creates sub-blocks) */
        if (a > 100) {
            b = b * 2;
        } else {
            b = b / 2;
        }
        
        /* More modifications to original test variables */
        a = a ^ b;
        b = b + glob_b;
        
        /* Function call with side effect */
        modify(&a);
        
        sink = a - b;
    }
    
    sink = a + b;
}

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
    test_multi_modification(arg1, arg2, arg3, arg4);
    result += sink;
    
    /* Test 3: Indirect modification */
    int data = 50;
    test_indirect_modification(&data, 40);
    result += sink + data;
    
    /* Test 4: Loop nested */
    test_loop_nested(10);
    result += glob_a + glob_b;
    
    /* Test 5: Complex condition */
    test_complex_condition(arg1, arg2);
    result += sink;
    
    /* Test 6: Volatile mix */
    test_volatile_mix();
    result += glob_a + glob_b;
    
    /* Test 7: Multi-BB simulation */
    test_multi_bb_simulation(arg3, arg4);
    result += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
