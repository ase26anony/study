/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targeting lines 577-583: modified_in_p check in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink;  /* To prevent dead code elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() % 100;
}

static void __attribute__((noinline, noipa)) dummy_use(int x) {
    sink = x;
}

static void __attribute__((noinline, noipa)) modify(int *x) {
    *x = *x + 1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Condition uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in the condition */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b * 2;
        dummy_use(a + b);
        /* Another modification to ensure multiple instructions */
        a = a ^ 0xFF;
    }
    
    dummy_use(a);
    dummy_use(b);
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    int w = get_value();
    
    /* Compound condition using multiple variables */
    if ((x > y) || (z < w) || (glob_a != glob_b)) {
        /* Modify multiple variables from the condition */
        x = x * 3;
        y = y - 5;
        /* Include arithmetic operations */
        z = (z << 2) | 1;
        w = w % 17;
        /* More instructions to flesh out the basic block */
        dummy_use(x + y + z + w);
        x = x ^ y;
    }
    
    dummy_use(x);
    dummy_use(y);
    dummy_use(z);
    dummy_use(w);
}

/* Test case 3: Indirect modification via pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int val1 = get_value();
    int val2 = get_value();
    int *ptr1 = &val1;
    int *ptr2 = &val2;
    
    /* Condition uses pointer values */
    if (ptr1 != NULL && *ptr1 > *ptr2) {
        /* Indirect modification through pointer */
        *ptr1 = *ptr1 + 42;
        /* Additional operations */
        val2 = val2 * 3;
        dummy_use(*ptr1 + val2);
        /* Another indirect modification */
        modify(ptr1);
    }
    
    dummy_use(val1);
    dummy_use(val2);
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int threshold = glob_b;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Condition uses loop-varying variables */
        if (counter > threshold && glob_c != 0) {
            /* Modify variable used in condition */
            counter = counter - 1;
            /* Additional operations to create more instructions */
            accumulator = accumulator + (counter * i);
            dummy_use(accumulator);
            /* Another modification */
            threshold = threshold ^ i;
        }
        
        /* Loop update with external dependency */
        counter = counter + get_value() % 5;
        dummy_use(counter);
    }
    
    dummy_use(accumulator);
}

/* Test case 5: Complex condition with function calls */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = get_value();
    int b = get_value();
    int c = get_value();
    
    /* Complex condition that can't be easily simplified */
    if ((a > b) ? (c != 0) : (b > a)) {
        /* Modify variables from condition */
        a = a + b;
        b = b - c;
        c = c * 2;
        /* Sequence of operations */
        a = a | 0x1;
        b = b & 0xFF;
        dummy_use(a + b + c);
        /* Function call that might modify */
        modify(&a);
    }
    
    dummy_use(a);
    dummy_use(b);
    dummy_use(c);
}

/* Test case 6: Volatile accesses in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mod(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    int local1, local2;
    
    /* Read volatiles into locals for condition */
    local1 = v1;
    local2 = v2;
    
    if (local1 > local2 || glob_c < glob_d) {
        /* Modify the local variables used in condition */
        local1 = local1 * 2;
        local2 = local2 + 5;
        /* Write back to volatiles */
        v1 = local1;
        v2 = local2;
        /* Additional arithmetic */
        int temp = local1 ^ local2;
        dummy_use(temp);
        local1 = local1 % 13;
    }
    
    dummy_use(local1);
    dummy_use(local2);
}

int main(void) {
    int checksum = 0;
    
    /* Seed random for get_value() */
    srand(42);
    
    /* Execute all test cases */
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
    
    test_volatile_mod();
    checksum += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    /* Also use the globals to prevent removal */
    printf("Globals: %d %d %d %d\n", glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
