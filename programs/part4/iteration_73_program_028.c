/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583: modified_in_p check in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To consume results and prevent elimination */

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) use_value(int val) {
    sink ^= val;  /* Side effect to prevent elimination */
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return (a > b) ^ (sink & 1);
}

static int __attribute__((noinline, noipa)) modify(int x) {
    return x + (sink & 3) - 1;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Condition uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which appears in the test expression */
        a = a + 1;
        /* Additional non-debug instructions */
        b = b ^ 0x55;
        glob_c = glob_c - 1;
        use_value(a + b);
    }
    
    sink += a * b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Compound condition using multiple variables */
    if ((x > y) || (z < glob_d)) {
        /* Modify BOTH x and z which appear in the condition */
        x = modify(x);
        /* Additional arithmetic */
        y = y * 2;
        /* Modify z as well */
        z = z | 0x01;
        use_value(x + y + z);
    }
    
    sink += x - y + z;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(int *ptr) {
    int val = get_value();
    
    /* Condition tests the pointer and a value */
    if (ptr != NULL && val > 50) {
        /* Indirect modification - changes what ptr points to */
        *ptr = 42;
        /* Also modify val which is in the condition */
        val = val / 2;
        /* Additional operations to flesh out the basic block */
        int temp = *ptr + val;
        use_value(temp);
    }
    
    if (ptr != NULL) {
        sink += *ptr;
    }
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int threshold = glob_b;
    
    for (int i = 0; i < 10; i++) {
        /* Condition uses variables that change in the loop */
        if (counter > threshold && glob_c > 0) {
            /* Modify counter which is in the condition */
            counter = counter - 1;
            /* Additional statements */
            threshold = threshold + (i & 1);
            glob_c = glob_c >> 1;
            use_value(counter + threshold);
        }
        
        /* Loop-carried dependency */
        counter = counter + (i & 3);
        sink += counter;
    }
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = get_value();
    int b = get_value();
    int c = get_value();
    
    /* Complex condition with function call */
    if (cond_check(a, b) && (c % 2 == 0)) {
        /* Modify 'a' which is used in cond_check */
        a = a ^ b;  /* Bitwise operation */
        /* Modify 'c' which is also in the condition */
        c = c + 1;
        /* Additional arithmetic to ensure non-debug insns */
        int temp = (a << 2) | (c & 0xF);
        use_value(temp);
    }
    
    sink += a * 100 + b * 10 + c;
}

/* Test case 6: Multiple modifications in sequence */
static void __attribute__((noinline, noipa)) test_sequence_mod(void) {
    int x = glob_a;
    int y = glob_b;
    volatile int* volatile_ptr = &glob_c;
    
    /* Condition with multiple variables */
    if (x != y || *volatile_ptr > 0) {
        /* Sequence of modifications (at least 2-3 assignments) */
        x = x * 3 + 1;          /* First modification of test expr variable */
        y = y ^ x;              /* Second modification of test expr variable */
        *volatile_ptr = x + y;  /* Third modification (indirect) */
        
        /* Additional non-trivial computation */
        int result = (x & 0xFF) << 8 | (y & 0xFF);
        for (int i = 0; i < 3; i++) {
            result = result ^ (1 << i);
        }
        use_value(result);
    }
    
    sink += x - y;
}

int main(void) {
    int dummy = 0;
    int *ptr = &dummy;
    
    /* Seed RNG for get_value() */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    test_multi_var_mod();
    test_indirect_mod(ptr);
    test_loop_nested();
    test_complex_cond();
    test_sequence_mod();
    
    /* Additional test with different pointer scenario */
    int local_var = 100;
    test_indirect_mod(&local_var);
    
    /* Print checksum to ensure all code executed */
    printf("Result checksum: %d\n", sink);
    printf("Local var modified: %d\n", local_var);
    
    return 0;
}
