/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583 checking if test_expr is modified in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink for results to prevent elimination */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink += val;
}

static int __attribute__((noinline, noipa)) cond_check(int x, int y) {
    return x > y;
}

static int __attribute__((noinline, noipa)) modify_value(int x) {
    return x * 2 + 1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b - 1;
        glob_c = glob_c * 2;
        /* Use volatile sink to prevent elimination */
        use_value(a + b + glob_c);
    }
    
    /* Ensure values are used */
    use_value(a);
    use_value(b);
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Compound conditional using multiple variables */
    if ((x > y) || (z < glob_d) || (x + y > z)) {
        /* Modify multiple variables from test expression */
        x = x * 3;      /* x appears in (x > y) and (x + y > z) */
        y = y / 2;      /* y appears in (x > y) and (x + y > z) */
        z = z + 10;     /* z appears in (z < glob_d) */
        
        /* Additional arithmetic to flesh out basic block */
        int temp = x ^ y;
        temp = temp | z;
        use_value(temp);
        
        /* More modifications to ensure multiple instructions */
        x = x & 0xFF;
        y = y | 0x7F;
    }
    
    use_value(x + y + z);
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int val1 = get_value();
    int val2 = get_value();
    int *ptr1 = &val1;
    int *ptr2 = &val2;
    
    /* Test expression uses pointer values */
    if (ptr1 != NULL && *ptr1 > *ptr2) {
        /* Indirect modification through pointer from test expression */
        *ptr1 = *ptr1 + 5;      /* Modifies val1 which is used in *ptr1 > *ptr2 */
        *ptr2 = *ptr2 - 3;      /* Modifies val2 which is used in *ptr1 > *ptr2 */
        
        /* Additional operations */
        int sum = *ptr1 + *ptr2;
        use_value(sum);
        
        /* More pointer arithmetic */
        ptr1 = &val2;  /* Change pointer itself */
        *ptr1 = 100;
    }
    
    use_value(val1);
    use_value(val2);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        int local_var = get_value() + i;
        
        /* Test expression uses loop-varying variables */
        if (counter > local_var && accumulator < 100) {
            /* Modify variables from test expression */
            counter = counter - 1;          /* counter appears in test */
            accumulator = accumulator + 5;  /* accumulator appears in test */
            
            /* Additional statements in then block */
            local_var = local_var * 2;
            use_value(local_var);
            
            /* More arithmetic to ensure multiple instructions */
            int temp = counter ^ accumulator;
            temp = temp << 2;
            use_value(temp);
        }
        
        /* Loop-carried dependency prevents optimization */
        accumulator = accumulator + counter;
    }
    
    use_value(counter);
    use_value(accumulator);
}

/* Test case 5: Complex condition with function call in test */
static void __attribute__((noinline, noipa)) test_func_in_cond(void) {
    int a = glob_b;
    int b = glob_c;
    int c = glob_d;
    
    /* Function call in condition */
    if (cond_check(a, b) && c != 0) {
        /* Modify variables used in test expression */
        a = modify_value(a);  /* a appears in cond_check(a, b) */
        b = b * 2;            /* b appears in cond_check(a, b) */
        c = c / 2;            /* c appears in c != 0 */
        
        /* Sequence of assignments to create multiple instructions */
        int t1 = a + b;
        int t2 = b + c;
        int t3 = a + c;
        use_value(t1 + t2 + t3);
        
        /* More modifications */
        a = a | 0x01;
        b = b & 0xFE;
    }
    
    use_value(a + b + c);
}

/* Test case 6: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_in_test(void) {
    volatile int v1 = get_value();
    volatile int v2 = get_value();
    int normal = get_value();
    
    /* Test uses volatile variables */
    if (v1 > v2 && normal > 0) {
        /* Modify the normal variable from test expression */
        normal = normal * 3 + 7;
        
        /* Also modify through volatile pointers */
        int *p = (int*)&v1;
        *p = *p + 1;  /* Indirect modification of volatile */
        
        /* Multiple instructions */
        int result = normal + v1 + v2;
        use_value(result);
        
        normal = normal ^ 0x55;
    }
    
    use_value(normal);
    use_value(v1);
    use_value(v2);
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    
    /* Seed random for variability */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    test_multi_var_mod();
    test_indirect_mod();
    test_loop_nested();
    test_func_in_cond();
    test_volatile_in_test();
    
    /* Aggregate results and print checksum */
    result = sink;
    printf("Result checksum: %d\n", result);
    printf("All test cases executed.\n");
    
    return 0;
}
