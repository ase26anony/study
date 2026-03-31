#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int counter = 0;
volatile int limit = 100;
volatile int x = 0, y = 1, z = 2;

/* Global variable for function call side effects */
volatile int global_modifier = 0;

/* Function with side effects */
int __attribute__((noinline)) side_effect_func(void) {
    return global_modifier++;
}

/* Test function 1: Simple modification in then block */
void __attribute__((noinline)) test_simple_modification(void) {
    for (int i = 0; i < 100; i++) {
        /* The condition uses 'a' and 'b', and the then block modifies 'a' */
        if (a > b) {
            /* This assignment modifies 'a' which is part of the condition */
            a = b + i;  // Modifies condition variable early in then block
            c = d * 2;  // Additional instruction to create block header
            d = c + 1;  // Another instruction for the header
        }
        /* Prevent loop unrolling from simplifying too much */
        if (i % 10 == 0) {
            b = b + 1;
        }
    }
}

/* Test function 2: Compound condition with multiple modifications */
void __attribute__((noinline)) test_compound_condition(void) {
    volatile int local_x = x, local_y = y, local_z = z;
    
    for (int i = 0; i < 50; i++) {
        /* Complex condition with && */
        if (local_x != 0 && local_y < local_z) {
            /* Modify variables used in the condition */
            local_x = local_y;  // Modifies first part of condition
            local_y = local_z + i;  // Modifies second part of condition
            local_z = local_x * 2;  // Additional modification
        }
        
        /* Add some arithmetic to prevent dead code elimination */
        local_x = local_x + (i & 1);
        local_y = local_y - (i & 2);
    }
    
    x = local_x;
    y = local_y;
    z = local_z;
}

/* Test function 3: Static variable modified in then block */
void __attribute__((noinline)) test_static_modification(void) {
    static volatile int static_counter = 0;
    
    for (int i = 0; i < 75; i++) {
        /* Condition uses static variable */
        if (static_counter < limit) {
            /* Modify the static variable used in condition */
            static_counter++;  // Direct modification of condition variable
            int temp = static_counter * 2;
            static_counter = temp / 2;  // Another modification
        }
        
        /* Mix in some function calls */
        if (i % 15 == 0) {
            limit = limit + side_effect_func();
        }
    }
    
    counter = static_counter;
}

/* Test function 4: Function call in condition with side effects */
void __attribute__((noinline)) test_function_in_condition(void) {
    volatile int result1, result2;
    
    for (int i = 0; i < 30; i++) {
        /* Function call in condition */
        result1 = side_effect_func();
        result2 = side_effect_func();
        
        if (result1 > 0 && result2 < 10) {
            /* Modify global variable that affects future function calls */
            global_modifier = result1 + result2;  // Affects side_effect_func
            result1 = global_modifier - i;
            result2 = global_modifier + i;
        }
        
        /* Additional computation to create basic block structure */
        a = a + result1;
        b = b + result2;
    }
}

/* Test function 5: Nested conditions with modifications */
void __attribute__((noinline)) test_nested_modifications(void) {
    volatile int p = a, q = b, r = c;
    
    for (int i = 0; i < 25; i++) {
        if (p > q) {
            /* Outer then block modifies p */
            p = q + i;
            
            if (r < p) {
                /* Inner then block modifies r (used in inner condition)
                   and also modifies p (used in outer condition) */
                r = p * 2;
                p = r / 2;  // This modifies outer condition variable
                q = q + 1;  // Additional modification
            }
        }
        
        /* Ensure variables change each iteration */
        p = p + (i % 3);
        q = q - (i % 2);
        r = r + (i % 4);
    }
    
    a = p;
    b = q;
    c = r;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    if (argc > 1) {
        a = atoi(argv[1]);
        if (argc > 2) b = atoi(argv[2]);
        if (argc > 3) c = atoi(argv[3]);
        if (argc > 4) d = atoi(argv[4]);
    }
    
    printf("Initial values: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("counter=%d, limit=%d, global_modifier=%d\n", counter, limit, global_modifier);
    
    /* Call all test functions to exercise different patterns */
    test_simple_modification();
    printf("After test_simple_modification: a=%d, b=%d\n", a, b);
    
    test_compound_condition();
    printf("After test_compound_condition: x=%d, y=%d, z=%d\n", x, y, z);
    
    test_static_modification();
    printf("After test_static_modification: counter=%d, limit=%d\n", counter, limit);
    
    test_function_in_condition();
    printf("After test_function_in_condition: global_modifier=%d\n", global_modifier);
    
    test_nested_modifications();
    printf("After test_nested_modifications: a=%d, b=%d, c=%d\n", a, b, c);
    
    /* Compute and print final result */
    int final_result = a + b + c + d + x + y + z + counter + global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result > 1000 ? 0 : 1;
}
