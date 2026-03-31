/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p detection in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink; /* To prevent optimization */

/* Dummy no-inline functions to prevent optimization */
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
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in test expression */
        a = a + 1;           /* Direct modification */
        a = modify_value(a); /* Function call modification */
        b = b ^ 0x55;        /* Modify other test variable */
        use_value(a + b);    /* Use results */
    }
    
    /* Ensure variables are used */
    use_value(a);
    use_value(b);
}

/* Test case 2: Multiple variables in compound conditional */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Complex test expression */
    if ((x > y) || (z < glob_d) || (glob_a != glob_b)) {
        /* Modify multiple test expression variables */
        x = x * 3;           /* Arithmetic modification */
        y = y | 0xF0;        /* Bitwise modification */
        z = z + glob_c;      /* Using global in modification */
        
        /* Additional statements to flesh out basic block */
        int temp = x * y;
        temp = temp >> 2;
        use_value(temp + z);
    }
    
    use_value(x + y + z);
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int value = get_value();
    int *ptr = &value;
    int threshold = 100;
    
    /* Test expression uses value */
    if (value > threshold && ptr != NULL) {
        /* Indirect modification through pointer */
        *ptr = 42;           /* Modifies 'value' indirectly */
        value = value + 10;  /* Direct modification */
        
        /* Additional computation */
        int result = (*ptr) * 2;
        use_value(result);
    }
    
    use_value(value);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int limit = glob_b;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Test expression uses loop-varying variables */
        if (counter < limit && (i & 1)) {
            /* Modify test expression variable */
            counter = counter + i;      /* Loop-carried modification */
            accumulator = accumulator ^ counter;
            
            /* Additional statements */
            int temp = counter * i;
            use_value(temp);
        }
        
        /* Ensure loop has side effects */
        limit = limit - 1;
        use_value(accumulator);
    }
    
    use_value(counter + accumulator);
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = get_value();
    int b = get_value();
    int c = get_value();
    
    /* Condition with function call */
    if (cond_check(a, b) || (c % 2 == 0)) {
        /* Modify variables used in condition */
        a = a ^ b;          /* Uses both a and b */
        b = b + c;          /* Uses b and c */
        c = modify_value(c); /* Function call */
        
        /* Multiple arithmetic operations */
        int result = (a * b) / (c + 1);
        result = result | 0xAA;
        use_value(result);
    }
    
    use_value(a + b + c);
}

/* Test case 6: Volatile access in test expression */
static void __attribute__((noinline, noipa)) test_volatile_cond(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    int normal = get_value();
    
    /* Test expression with volatile reads */
    if (v1 > v2 || normal > 50) {
        /* Modify the normal variable */
        normal = normal * 2;
        v1 = v1 + 1;  /* This might not be detected as modifying test_expr
                         if test_expr only contains the values at condition time */
        
        /* Use volatile write to prevent elimination */
        sink = normal;
        use_value(v1 + v2);
    }
    
    use_value(normal);
}

/* Main function that exercises all test cases */
int main(void) {
    int checksum = 0;
    
    /* Seed random for reproducibility */
    srand(42);
    
    /* Execute all test cases multiple times */
    for (int i = 0; i < 3; i++) {
        test_single_var_mod();
        checksum += sink;
        
        test_multi_var_mod();
        checksum += sink;
        
        test_indirect_mod();
        checksum += sink;
        
        test_loop_nested();
        checksum += sink;
        
        test_complex_cond();
        checksum += sink;
        
        test_volatile_cond();
        checksum += sink;
        
        /* Modify globals to change conditions */
        glob_a += i;
        glob_b -= i;
        glob_c ^= i;
    }
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
