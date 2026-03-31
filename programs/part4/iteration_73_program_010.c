/* Test program for if-conversion uncovered lines in ifcvt.cc
 * Specifically targets lines 577-583 checking if test_expr is modified in then_bb
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent dead code elimination */

/* Non-inlineable functions to create opaque operations */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_cond(int a, int b) {
    return a > b;
}

static void __attribute__((noinline, noipa)) dummy_use(int val) {
    sink = val;
}

/* Test Case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b ^ 0x55;
        glob_c = glob_c * 2;
        dummy_use(a + b);
    }
    
    sink = a + b + glob_c;
}

/* Test Case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Compound conditional using multiple variables */
    if ((x > y) || (z < glob_d)) {
        /* Modify both x and z which appear in test expression */
        x = x * 3;      /* modifies first part of OR condition */
        z = z | 0x0F;   /* modifies second part of OR condition */
        
        /* Additional arithmetic to create more instructions */
        y = y + (x >> 2);
        glob_d = glob_d - 1;
        
        dummy_use(x + y + z);
    }
    
    sink = x ^ y ^ z;
}

/* Test Case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int val1 = get_value();
    int val2 = get_value();
    int *ptr1 = &val1;
    int *ptr2 = &val2;
    
    /* Test expression uses pointer comparison */
    if (ptr1 != NULL && *ptr1 > val2) {
        /* Indirect modification through pointer dereference */
        *ptr1 = *ptr1 + 5;  /* modifies what ptr1 points to */
        val2 = val2 * 2;    /* direct modification */
        
        /* Additional operations */
        ptr2 = &glob_a;
        *ptr2 = *ptr2 + 1;
        
        dummy_use(*ptr1 + val2);
    }
    
    sink = val1 + val2;
}

/* Test Case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = get_value() % 100;
    int threshold = 50;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Test expression uses loop-varying variables */
        if (counter > threshold && glob_a < glob_b) {
            /* Modify counter which is in test expression */
            counter = counter - 1;
            /* Additional modifications */
            threshold = threshold + (i & 1);
            glob_a = glob_a + i;
            
            /* Complex enough to avoid simplification */
            accumulator += (counter * threshold) >> 1;
        }
        
        /* Loop update that depends on modified variables */
        counter += (accumulator % 3);
        dummy_use(counter);
    }
    
    sink = counter + accumulator;
}

/* Test Case 5: Function call that modifies test expression variable */
static void __attribute__((noinline, noipa)) test_func_mod(void) {
    int a = glob_b;
    int b = glob_c;
    
    /* Use function call in condition */
    if (check_cond(a, b)) {
        /* Call function that modifies 'a' through pointer */
        modify(&a);
        
        /* Additional direct modifications */
        b = b ^ a;
        a = a << 1;  /* Another modification of test expression variable */
        
        /* Mix of operations */
        int temp = a * b;
        glob_d = glob_d ^ temp;
        
        dummy_use(temp);
    }
    
    sink = a - b;
}

/* Test Case 6: Complex test expression with multiple modifications */
static void __attribute__((noinline, noipa)) test_complex_expr(void) {
    int p = get_value();
    int q = get_value();
    int r = get_value();
    int s = get_value();
    
    /* Complex conditional expression */
    if ((p > q) && (r < s) && ((p + r) > (q + s))) {
        /* Modify multiple variables from test expression */
        p = p / 2;      /* appears in (p > q) and (p + r) > (q + s) */
        r = r * 3;      /* appears in (r < s) and (p + r) > (q + s) */
        s = s - p;      /* appears in (r < s) and (p + r) > (q + s) */
        
        /* Additional computations to create more instructions */
        q = q ^ r ^ s;
        int t = (p << 3) | (q & 0xFF);
        
        dummy_use(t);
    }
    
    sink = p + q + r + s;
}

int main(void) {
    int result = 0;
    
    /* Seed random for variability */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    result ^= sink;
    
    test_multi_var_mod();
    result ^= sink;
    
    test_indirect_mod();
    result ^= sink;
    
    test_loop_nested();
    result ^= sink;
    
    test_func_mod();
    result ^= sink;
    
    test_complex_expr();
    result ^= sink;
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
