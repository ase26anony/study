/* Test program for if-conversion uncovered lines in ifcvt.cc.gcov
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

/* Opaque, non-inlineable functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return a > b;
}

static void __attribute__((noinline, noipa)) noop(void) {
    asm volatile("" : : : "memory");
}

/* Test Case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Condition uses 'a' and 'b' */
    if (a > b && glob_c != 0) {
        /* Modify 'a' which is used in the test expression */
        a = a + 1;           /* Direct modification */
        a = a * 2;           /* Additional computation */
        b = b - 1;           /* Modify other test variable */
        noop();              /* Prevent optimization */
        sink = a + b;        /* Use result to prevent elimination */
    }
    sink = a;  /* Ensure value is used */
}

/* Test Case 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    int d = glob_d;
    
    /* Complex compound condition */
    if ((a > b || c < d) && (a != 0 || b != 0)) {
        /* Modify multiple test expression variables */
        a = a ^ 0x55;        /* Bitwise operation on test variable */
        b = b + c;           /* Use another test variable in computation */
        c = c * 2;           /* Modify second part of OR condition */
        d = d >> 1;          /* Modify variable from first condition */
        
        /* Additional statements to create more instructions */
        int temp = a * b;
        temp = temp + c - d;
        sink = temp;
        noop();
    }
    sink = a + b + c + d;
}

/* Test Case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int x = glob_a;
    int y = glob_b;
    int *ptr = &x;
    
    /* Condition based on pointer and value */
    if (ptr != NULL && x > y) {
        /* Indirect modification of test expression variable */
        *ptr = 42;           /* Modifies 'x' which is in test expression */
        y = y * 2;           /* Also modify other test variable */
        
        /* Additional operations to ensure basic block has multiple insns */
        int z = *ptr + y;
        z = z & 0xFF;
        sink = z;
        noop();
    }
    sink = x + y;
}

/* Test Case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int a = glob_a;
    int b = glob_b;
    int i;
    
    for (i = 0; i < 10; i++) {
        /* Condition uses variables modified in loop */
        if (a > b && i < 8) {
            /* Modify test expression variables */
            a = a + i;       /* Loop-carried modification */
            b = b - 1;       /* Direct modification */
            
            /* Additional computations */
            int temp = a * b;
            temp = temp >> 2;
            sink = temp;
            noop();
        }
        /* Ensure loop has side effects */
        sink = i;
    }
    sink = a + b;
}

/* Test Case 5: Function call that modifies test expression */
static void __attribute__((noinline, noipa)) test_func_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Use function in condition */
    if (check_condition(a, b) && a != 0) {
        /* Call function that modifies test variable */
        modify(&a);          /* 'a' is modified via pointer */
        b = get_value();     /* 'b' gets new value */
        
        /* Plain assignments to ensure modified_in_p detection */
        a = a | 0x01;        /* Bitwise OR */
        b = b & 0xFE;        /* Bitwise AND */
        
        /* Multiple statements */
        int result = a * 3 + b / 2;
        sink = result;
        noop();
    }
    sink = a - b;
}

/* Test Case 6: Complex arithmetic in then block */
static void __attribute__((noinline, noipa)) test_complex_arithmetic(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    
    /* Multi-variable condition */
    if (a > b || (b < c && a != c)) {
        /* Complex modification chain */
        a = (a * 3 + 7) / 2;     /* Arithmetic on test variable */
        b = b ^ a;               /* XOR with modified variable */
        c = c + (a << 2);        /* Shift operation involvement */
        
        /* Sequence of operations */
        int t1 = a + b;
        int t2 = t1 * c;
        t2 = t2 % 256;
        sink = t2;
        noop();
        
        /* Additional modification */
        a = a + t2;
    }
    sink = a + b + c;
}

/* Test Case 7: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_mix(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    int reg1 = v1;
    int reg2 = v2;
    
    /* Condition with volatile-derived values */
    if (reg1 > reg2 || reg1 == 0) {
        /* Modify the register copies */
        reg1 = reg1 + v2;     /* Mix with volatile */
        reg2 = reg2 * 2;
        
        /* Multiple assignments */
        int sum = reg1 + reg2;
        sum = sum * sum;
        sink = sum;
        noop();
        
        /* Further modification */
        reg1 = reg1 | 0x80;
    }
    v1 = reg1;  /* Store back to volatile */
    v2 = reg2;
    sink = v1 + v2;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize random seed for non-deterministic values */
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
    
    test_func_mod();
    checksum += sink;
    
    test_complex_arithmetic();
    checksum += sink;
    
    test_volatile_mix();
    checksum += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    /* Verify by checking if we modified globals */
    printf("Global values: %d, %d, %d, %d\n", 
           glob_a, glob_b, glob_c, glob_d);
    
    return 0;
}
