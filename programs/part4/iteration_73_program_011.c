/* Test program for if-conversion uncovered lines in ifcvt.cc */
/* Specifically targets lines 577-583 checking for modifications of test_expr in then_bb */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent constant propagation */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* Sink for results to prevent elimination */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) get_value(void) {
    return rand() & 0xFF;
}

static void __attribute__((noinline, noipa)) modify(int *ptr) {
    if (ptr) *ptr += 1;
}

static int __attribute__((noinline, noipa)) cond_check(int a, int b) {
    return a > b;
}

static void __attribute__((noinline, noipa)) dummy_use(int val) {
    sink += val;
}

/* Test case 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Test expression uses a and b */
    if (a > b && glob_c != 0) {
        /* Modify variable 'a' used in test expression */
        a = a + 1;                     /* Direct modification */
        b = b * 2;                     /* Additional statement */
        a = a ^ 0x55;                  /* Another modification of test variable */
        dummy_use(a + b);              /* Prevent elimination */
    }
    
    sink += a + b;
}

/* Test case 2: Multiple variables from compound conditional modified */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = get_value();
    int y = get_value();
    int z = get_value();
    
    /* Compound conditional using multiple variables */
    if ((x > y) || (z < glob_d) || (glob_a != 0)) {
        /* Modify multiple variables from test expression */
        x = x * 3 + 1;                 /* Modify x from first part of condition */
        z = z | 0xF0;                  /* Modify z from second part of condition */
        y = y - 5;                     /* Additional modification */
        dummy_use(x + y + z);          /* Prevent elimination */
        
        /* More statements to flesh out the basic block */
        glob_a = glob_a ^ 1;           /* Modify global used in condition */
        sink += x * y;
    }
    
    sink += x + y + z;
}

/* Test case 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(void) {
    int val1 = get_value();
    int val2 = get_value();
    int *ptr1 = &val1;
    int *ptr2 = &val2;
    
    /* Test expression uses pointer values */
    if (ptr1 != NULL && *ptr1 > val2) {
        /* Indirect modification through pointer */
        *ptr1 = *ptr1 + 42;            /* Modifies what ptr1 points to */
        val2 = val2 * 2;               /* Direct modification of val2 */
        
        /* Additional operations */
        ptr2 = &val1;                  /* Change pointer */
        *ptr2 = *ptr2 - 10;            /* Another indirect modification */
        dummy_use(*ptr1 + val2);
    }
    
    sink += val1 + val2;
}

/* Test case 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        int temp = get_value();
        
        /* Condition uses loop-varying variables */
        if (counter > temp && accumulator < 100) {
            /* Modify variables used in test expression */
            counter = counter - 1;      /* Modifies counter from condition */
            accumulator = accumulator + temp * 2; /* Modifies accumulator */
            
            /* Additional statements */
            temp = temp ^ i;            /* Modify local temp */
            dummy_use(counter + accumulator);
        }
        
        /* Loop-carried dependency */
        accumulator += counter;
        sink += accumulator;
    }
}

/* Test case 5: Complex condition with function call */
static void __attribute__((noinline, noipa)) test_complex_cond(void) {
    int a = glob_a;
    int b = glob_b;
    int c = glob_c;
    
    /* Complex condition with function call */
    if (cond_check(a, b) && (c != 0 || a < 100)) {
        /* Modify test expression variables */
        a = modify(&a) ? a : a + 1;    /* Attempt modification through function */
        b = b << 2;                    /* Bitwise operation on b */
        c = c % 7;                     /* Modifies c from condition */
        
        /* Sequence of assignments */
        int t = a * b;
        t = t + c;
        t = t ^ 0xAA;
        dummy_use(t);
        
        /* Additional modification */
        a = a + t;
    }
    
    sink += a + b + c;
}

/* Test case 6: Multiple basic blocks within then */
static void __attribute__((noinline, noipa)) test_multi_bb_then(void) {
    int x = get_value();
    int y = get_value();
    
    if (x > 50 && y < 100) {
        /* First modification in then block */
        x = x + y;
        y = y * 2;
        
        /* Conditional inside then (creates another basic block) */
        if (x > 75) {
            x = x - 10;                /* Another modification */
            dummy_use(x);
        }
        
        /* Back to original then block */
        x = x | 0x0F;                  /* Final modification of test variable */
        y = y & 0xF0;
    }
    
    sink += x + y;
}

/* Test case 7: Volatile access in condition and modification */
static void __attribute__((noinline, noipa)) test_volatile_cond(void) {
    volatile int v1 = glob_a;
    volatile int v2 = glob_b;
    int normal = get_value();
    
    /* Condition uses volatile variables */
    if (v1 > v2 || normal > 0) {
        /* Modify variables - volatile writes have side effects */
        v1 = v1 + normal;              /* Modifies volatile from condition */
        normal = normal * 3;           /* Modifies normal variable */
        
        /* Additional volatile operation */
        sink = v1 + normal;            /* Volatile write */
        
        /* More arithmetic */
        v2 = v2 - 1;                   /* Modifies other volatile */
        dummy_use(normal);
    }
    
    sink += normal;
}

int main(void) {
    int result = 0;
    
    /* Seed random for reproducibility */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    result += sink;
    
    test_multi_var_mod();
    result += sink;
    
    test_indirect_mod();
    result += sink;
    
    test_loop_nested();
    result += sink;
    
    test_complex_cond();
    result += sink;
    
    test_multi_bb_then();
    result += sink;
    
    test_volatile_cond();
    result += sink;
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", result);
    
    return 0;
}
