/* Test program for ifcvt.cc uncovered lines 577-583 */
#include <stdio.h>
#include <stdlib.h>

/* Global variables for test expressions */
volatile int glob_a = 10;
volatile int glob_b = 20;
volatile int glob_c = 30;
volatile int glob_d = 40;
volatile int sink = 0;  /* To prevent optimization */

/* Opaque, non-inlineable functions */
static int __attribute__((noinline, noipa)) check_condition(int a, int b) {
    return a > b;
}

static int __attribute__((noinline, noipa)) modify_value(int x) {
    return x + rand() % 10;
}

static void __attribute__((noinline, noipa)) dummy_side_effect(int *p) {
    *p += 1;
}

/* Test 1: Single variable modification in then block */
static void __attribute__((noinline, noipa)) test_single_var_mod(void) {
    int a = glob_a;
    int b = glob_b;
    
    /* Complex condition using function call */
    if (check_condition(a, b) && (glob_c != 0)) {
        /* Modify variable used in condition */
        a = a + 1;                    /* Direct modification */
        a = a * 2;                    /* Additional computation */
        glob_c = glob_c - 1;          /* Modify another test variable */
        dummy_side_effect(&a);        /* Function call with side effect */
        sink += a + glob_c;           /* Prevent dead code elimination */
    }
}

/* Test 2: Multiple variables from compound conditional */
static void __attribute__((noinline, noipa)) test_multi_var_mod(void) {
    int x = glob_a;
    int y = glob_b;
    int z = glob_c;
    int w = glob_d;
    
    /* Compound conditional with multiple variables */
    if ((x > y) || (z < w) || (x != 0)) {
        /* Modify multiple test expression variables */
        x = x | 0xFF;                 /* Bitwise operation */
        y = y * 3;                    /* Arithmetic */
        z = modify_value(z);          /* Function call */
        w = w ^ 0xAA;                 /* Another bitwise op */
        
        /* Additional statements to flesh out basic block */
        int temp = x + y;
        temp = temp << 2;
        sink += temp + z + w;
    }
}

/* Test 3: Indirect modification through pointer */
static void __attribute__((noinline, noipa)) test_indirect_mod(int *ptr) {
    int local = glob_a;
    
    /* Test expression uses local variable */
    if (local > 5 && ptr != NULL) {
        /* Indirect modification - could affect aliased variables */
        *ptr = 42;                    /* Modify through pointer */
        local = local + *ptr;         /* Use modified value */
        
        /* Additional computations */
        for (int i = 0; i < 3; i++) {
            local = local ^ (1 << i);
        }
        sink += local;
    }
}

/* Test 4: Nested in loop with loop-carried dependency */
static void __attribute__((noinline, noipa)) test_loop_nested(void) {
    int counter = glob_a;
    int threshold = glob_b;
    
    for (int i = 0; i < 10; i++) {
        /* Condition uses loop-varying variables */
        if (counter > threshold && glob_c != 0) {
            /* Modify test expression variables */
            counter = counter - 1;     /* Affects loop condition */
            threshold = threshold + i; /* Affects next iteration */
            glob_c = glob_c >> 1;      /* Right shift */
            
            /* Additional statements */
            int result = counter * threshold;
            sink += result;
        } else {
            counter = counter + 2;
        }
        
        /* Loop-carried dependency */
        dummy_side_effect(&counter);
    }
}

/* Test 5: Complex arithmetic in then block */
static void __attribute__((noinline, noipa)) test_complex_arithmetic(void) {
    int a = glob_a;
    int b = glob_b;
    volatile int vol = glob_c;
    
    /* Condition with volatile to prevent constant folding */
    if (a > b && vol != 0) {
        /* Multiple modifications of test variables */
        a = (a * 3) / 2;              /* Complex arithmetic */
        b = b ^ a;                    /* Depends on modified a */
        vol = vol - 1;                /* Modify volatile */
        
        /* Chain of computations */
        int t1 = a << 3;
        int t2 = b >> 1;
        int t3 = t1 | t2;
        sink += t3 + vol;
        
        /* Function call that might modify globals */
        dummy_side_effect(&glob_d);
    }
}

/* Test 6: Mixed statements with function calls */
static void __attribute__((noinline, noipa)) test_mixed_statements(int param) {
    int x = param;
    int y = glob_b;
    
    /* Non-trivial condition */
    if (x != 0 && y > 10) {
        /* Sequence of different statement types */
        x = x & 0x0F;                 /* Bitwise AND */
        y = modify_value(y);          /* Function call */
        
        /* Intermediate computation */
        int z = x * y;
        z = z % 256;
        
        /* Another modification of test variable */
        x = x + z;
        
        /* Volatile access */
        sink += x + y + glob_d;
        
        /* Additional dummy call */
        dummy_side_effect(&x);
    }
}

int main(void) {
    int array[10];
    int *ptr = &array[0];
    
    /* Initialize random seed for modify_value */
    srand(42);
    
    /* Execute all test cases */
    test_single_var_mod();
    test_multi_var_mod();
    test_indirect_mod(ptr);
    test_loop_nested();
    test_complex_arithmetic();
    test_mixed_statements(glob_a);
    
    /* Additional test with pointer derived from test variable */
    {
        int base = glob_a;
        int *derived_ptr = &base;
        
        if (derived_ptr != NULL && base > 0) {
            *derived_ptr = 100;        /* Indirect modification */
            base = base * 2;           /* Direct modification */
            sink += base;
        }
    }
    
    /* Print checksum to verify execution */
    printf("Checksum: %d\n", sink);
    
    return 0;
}
