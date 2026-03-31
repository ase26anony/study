#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int x = 0, y = 1, z = 2;
volatile int counter = 0;
static volatile int static_var = 100;

/* Non-pure function with side effects */
int __attribute__((noinline)) get_value() {
    static int internal = 0;
    return internal++ + (static_var % 10);
}

/* Function 1: Simple modification in then block */
void __attribute__((noinline)) test_simple_modification() {
    for (int i = 0; i < 10; i++) {
        /* Critical: 'a' is used in condition and modified in then block */
        if (a > b) {
            a = b;  /* This modifies 'a' which is part of the condition */
            x = x + 1;
        }
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : : : "memory");
    }
}

/* Function 2: Compound condition with multiple modifications */
void __attribute__((noinline)) test_compound_condition() {
    for (int i = 0; i < 8; i++) {
        /* Complex test_expr with && operator */
        if (a != 0 && b < c) {
            b = a;      /* Modifies 'b' used in condition */
            a = 0;      /* Modifies 'a' used in condition */
            c = c + 1;  /* Additional instruction in header */
        }
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
}

/* Function 3: Static variable modification */
void __attribute__((noinline)) test_static_modification() {
    volatile int limit = 5;
    
    while (counter < limit) {
        /* 'counter' is used in condition and modified in then block */
        if (counter < limit) {
            counter++;          /* Modifies the condition variable */
            static_var -= 2;    /* Modifies variable used by get_value() */
            y = y * 2;          /* Additional non-label instruction */
        }
        /* Add some noise to prevent pattern recognition */
        if (z > 0) {
            z = z - 1;
        }
    }
}

/* Function 4: Function call in condition with side effects */
void __attribute__((noinline)) test_function_call_condition() {
    for (int i = 0; i < 6; i++) {
        /* Function call in condition, then block modifies what it reads */
        if (get_value() > 0 && x < y) {
            static_var += 5;    /* Affects future get_value() calls */
            x = y;              /* Modifies 'x' used in condition */
            d = d / 2;          /* Another modifying instruction */
        }
        /* Prevent tail merging */
        asm volatile("" : : : "memory");
    }
}

/* Function 5: Nested conditions with early modifications */
void __attribute__((noinline)) test_nested_early_modification() {
    volatile int m = 3, n = 7;
    
    for (int i = 0; i < 12; i++) {
        /* Multiple variables in complex condition */
        if ((a < b || c > d) && m != n) {
            /* EARLY modification - should be in block header */
            a = a + 1;      /* Modifies 'a' from condition */
            b = b - 1;      /* Modifies 'b' from condition */
            
            /* Additional code to ensure header has multiple insns */
            if (m > 0) {
                m = m - 1;
            }
        } else {
            /* Else block to create proper diamond pattern */
            c = c + 2;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    if (argc > 1) {
        a = atoi(argv[1]);
        b = atoi(argv[2 % argc]);
    }
    
    printf("Starting values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("x=%d, y=%d, z=%d, counter=%d, static_var=%d\n", x, y, z, counter, static_var);
    
    /* Call all test functions to exercise different patterns */
    test_simple_modification();
    test_compound_condition();
    test_static_modification();
    test_function_call_condition();
    test_nested_early_modification();
    
    /* Compute and print result to ensure side effects are observable */
    int result = a + b + c + d + x + y + z + counter + static_var;
    printf("Final result: %d\n", result);
    printf("Final values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("x=%d, y=%d, z=%d, counter=%d, static_var=%d\n", x, y, z, counter, static_var);
    
    return result % 256;
}
