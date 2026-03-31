/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targeting lines 577-583: modified_in_p check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink = val;
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
    
    /* Complex condition to prevent constant folding */
    if (cond_check(a, b) && (a + b) > 0) {
        /* Modify variable used in condition */
        a = modify_value(a);
        /* Additional non-debug instructions */
        b = b ^ 0x55;
        int temp = a * b;
        use_value(temp);
        /* Another modification of test expression variable */
        a = a + 1;
    }
    
    use_value(a + b);
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    int d = glob_d;
    
    /* Compound conditional using multiple variables */
    if ((a > b) || (c < d) || ((a + c) > (b + d))) {
        /* Modify multiple variables from the test expression */
        a = a * 3;
        c = c - 5;
        /* Additional arithmetic to flesh out basic block */
        b = b | 0xF0;
        d = d & 0x0F;
        /* Use modified values */
        use_value(a + b + c + d);
        /* Further modification */
        a = a >> 1;
        c = c << 1;
    }
    
    use_value(a * b * c * d);
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(int *ptr) {
    int val = get_value();
    
    /* Test expression uses pointer-derived value */
    if (ptr != NULL && *ptr > val) {
        /* Modify through pointer - affects test expression */
        *ptr = *ptr + 10;
        /* Additional operations */
        val = val ^ *ptr;
        use_value(val);
        /* Another indirect modification */
        *ptr = *ptr / 2;
    }
    
    if (ptr != NULL) {
        use_value(*ptr);
    }
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Condition uses variables modified in loop */
        if (a > b && (a + i) < 100) {
            /* Modify test expression variables */
            a = a + i;
            b = b - i;
            /* Additional computation */
            int prod = a * b;
            use_value(prod);
            /* Another modification */
            a = a % 50;
        }
        sum += a + b;
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 3 == 0) {
            b = b + get_value();
        }
    }
    
    use_value(sum);
}

/* Test case 5: Complex arithmetic in condition with modification */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Complex condition with multiple variables */
    if ((x * y > z) || (x + y < z) || ((x ^ y) == z)) {
        /* Modify all variables used in condition */
        x = x + y;
        y = y * 2;
        z = z ^ 0xFF;
        /* Sequence of real instructions */
        int t1 = x << 2;
        int t2 = y >> 1;
        int t3 = z & 0x7F;
        use_value(t1 + t2 + t3);
        /* Additional modification */
        x = x % 100;
    }
    
    use_value(x + y + z);
}

/* Test case 6: Function call that modifies global used in condition */
static int __attribute__((noinline, noipa)) update_global(void) {
    glob_a = glob_a + 5;
    return glob_a;
}

static void __attribute__((noinline, noipa)) test_global_mod(void) {
    int local_a = glob_a;
    int local_b = glob_b;
    
    if (local_a > local_b && glob_c != 0) {
        /* Function call modifies global used indirectly */
        local_a = update_global();
        /* Direct modification */
        local_b = local_b * 3;
        /* Arithmetic sequence */
        int diff = local_a - local_b;
        use_value(diff);
        /* Another modification */
        local_a = local_a | 0x01;
    }
    
    use_value(local_a * local_b);
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-deterministic values */
    glob_a = get_value();
    glob_b = get_value();
    glob_c = get_value();
    glob_d = get_value();
    
    /* Execute all test cases */
    test_single_var_mod();
    result += sink;
    
    test_multi_var_mod();
    result += sink;
    
    int array[5] = {10, 20, 30, 40, 50};
    test_indirect_mod(&array[2]);
    result += sink;
    
    test_loop_nested();
    result += sink;
    
    test_complex_cond();
    result += sink;
    
    test_global_mod();
    result += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
