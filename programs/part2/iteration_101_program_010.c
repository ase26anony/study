#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int x = 0, y = 1, z = 2;
volatile int counter = 0;
volatile int limit = 100;

/* Global variable for function call side effects */
volatile int global_state = 0;

/* Function with side effects for condition tests */
int __attribute__((noinline)) side_effect_func(void) {
    return global_state++;
}

/* Test function 1: Simple modification in then block */
void __attribute__((noinline)) test_simple_modification(void) {
    for (int i = 0; i < 10; i++) {
        /* Condition variable 'a' modified in then block */
        if (a > b) {
            /* This modifies 'a' which is used in the condition */
            a = b + i;  // Line in header that modifies condition variable
            x = a * 2;  // Additional instruction in header
            y = x + 1;  // Another instruction in header
        }
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : : : "memory");
    }
}

/* Test function 2: Compound condition with multiple modifications */
void __attribute__((noinline)) test_compound_condition(void) {
    for (int i = 0; i < 5; i++) {
        /* Compound condition using multiple variables */
        if (a != 0 && b < c && d > x) {
            /* Modify 'a' which is used in the first part of condition */
            a = 0;  // Modifies condition variable
            /* Also modify 'b' used in second part */
            b = c - 1;  // Another modification
            z = a + b;  // Additional computation
        }
        /* Mix in some other operations */
        c += i;
        asm volatile("" : : : "memory");
    }
}

/* Test function 3: Static variable in condition modified in then block */
void __attribute__((noinline)) test_static_modification(void) {
    static volatile int local_counter = 0;
    
    for (int i = 0; i < 8; i++) {
        /* Condition uses static variable */
        if (local_counter < limit) {
            /* Modify the condition variable */
            local_counter++;  // Direct modification
            int temp = local_counter * 2;
            x = temp + y;  // Additional instructions
        }
        /* Add some loop variability */
        limit -= (i % 2);
        asm volatile("" : : : "memory");
    }
}

/* Test function 4: Function call in condition with side effects */
void __attribute__((noinline)) test_function_call_condition(void) {
    for (int i = 0; i < 6; i++) {
        /* Function call in condition */
        if (side_effect_func() > 0 && a < b) {
            /* Modify global state that function reads */
            global_state = a + b;  // Modifies what side_effect_func returns
            a = global_state / 2;  // Also modifies 'a' from condition
            b = a + 1;  // Chain of modifications
        }
        /* Add some noise */
        d += i;
        asm volatile("" : : : "memory");
    }
}

/* Test function 5: Nested conditions with modifications */
void __attribute__((noinline)) test_nested_modifications(void) {
    volatile int p = 1, q = 2, r = 3;
    
    for (int i = 0; i < 7; i++) {
        /* Complex nested logical operations */
        if ((a > b || c < d) && (x != y || z > 0)) {
            /* Modify multiple condition variables */
            if (a > b) {
                a = b - 1;  // Modifies 'a' from outer condition
                x = a * 2;  // Additional instruction
            }
            if (c < d) {
                c = d;  // Modifies 'c' from outer condition
                y = c + 1;  // Additional instruction
            }
            /* More instructions in the header */
            z = x + y;
        }
        /* Loop variation */
        p = q + r;
        asm volatile("" : : : "memory");
    }
}

/* Test function 6: Pointer aliasing could affect condition */
void __attribute__((noinline)) test_pointer_aliasing(void) {
    volatile int val1 = 10, val2 = 20;
    volatile int *ptr1 = &val1;
    volatile int *ptr2 = &val1;  // Same address!
    
    for (int i = 0; i < 4; i++) {
        /* Condition using dereferenced pointer */
        if (*ptr1 > 5 && *ptr2 < 30) {
            /* Modify through ptr2 - aliases ptr1! */
            *ptr2 = 25;  // Modifies what ptr1 points to
            val1 = *ptr1 + 1;  // Additional modification
            val2 = val1 * 2;  // More instructions
        }
        /* Change pointer target occasionally */
        if (i % 2 == 0) {
            ptr2 = &val2;
        } else {
            ptr2 = &val1;
        }
        asm volatile("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    if (argc > 1) {
        a = atoi(argv[1]);
        b = atoi(argv[2 % argc]);
    }
    
    printf("Starting IF-CONVERSION test...\n");
    printf("Initial values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    
    /* Run all test functions */
    test_simple_modification();
    printf("After test 1: a=%d, b=%d, x=%d, y=%d\n", a, b, x, y);
    
    test_compound_condition();
    printf("After test 2: a=%d, b=%d, c=%d, z=%d\n", a, b, c, z);
    
    test_static_modification();
    printf("After test 3: counter=%d, limit=%d, x=%d\n", counter, limit, x);
    
    test_function_call_condition();
    printf("After test 4: global_state=%d, a=%d, b=%d\n", global_state, a, b);
    
    test_nested_modifications();
    printf("After test 5: a=%d, c=%d, x=%d, y=%d, z=%d\n", a, c, x, y, z);
    
    test_pointer_aliasing();
    printf("After test 6: val1=%d, val2=%d\n", a, b);  // Reusing a,b as val1,val2
    
    /* Compute and print final result */
    int result = a + b + c + d + x + y + z + global_state + counter;
    printf("Final computed result: %d\n", result);
    
    /* Read from stdin to prevent dead code elimination */
    if (argc > 3) {
        int dummy;
        printf("Enter a number: ");
        scanf("%d", &dummy);
        result += dummy;
    }
    
    return result % 256;
}
