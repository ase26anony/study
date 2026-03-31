#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and ensure basic blocks are preserved */
#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Global variables to create dependencies */
VOLATILE int global_counter = 0;
VOLATILE int global_modifier = 1;

/* Function with side effects used in conditions */
NOINLINE int side_effect_func(void) {
    return global_counter++;
}

/* Test function 1: Simple modification of condition variable in then block */
NOINLINE void test_simple_modification(void) {
    VOLATILE int a = 10;
    VOLATILE int b = 20;
    VOLATILE int result = 0;
    
    /* Loop to create interesting block structure */
    for (int i = 0; i < 100; i++) {
        /* Condition where 'a' is tested */
        if (a > b) {
            /* MODIFICATION IN THEN BLOCK: 'a' is modified here */
            a = b + i;  // This modifies 'a' which is part of the condition
            result += a;
        } else {
            b = a + i;
            result -= b;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 10 == 0) {
            global_modifier++;
        }
    }
    
    /* Use result to prevent dead code elimination */
    global_counter += result;
}

/* Test function 2: Compound condition with multiple modifications */
NOINLINE void test_compound_condition(void) {
    VOLATILE int x = 5;
    VOLATILE int y = 15;
    VOLATILE int z = 25;
    VOLATILE int temp = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Complex compound condition */
        if (x < y && y < z && z > 0) {
            /* Multiple modifications of condition variables */
            x = y + i;      // Modifies 'x' used in first part of condition
            y = z - i;      // Modifies 'y' used in second part of condition
            temp += x * y;
            
            /* Additional instruction to ensure header has multiple insns */
            z = (z > 100) ? z : z + 1;
        } else {
            z = x + y;
            temp -= z;
        }
        
        /* Mix in function calls to create complex RTL */
        if (side_effect_func() % 7 == 0) {
            x += global_modifier;
        }
    }
    
    global_counter += temp;
}

/* Test function 3: Modification through pointer/array access */
NOINLINE void test_pointer_modification(void) {
    VOLATILE int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    VOLATILE int idx = 0;
    VOLATILE int limit = 5;
    VOLATILE int sum = 0;
    
    /* Loop with condition that uses array element */
    while (idx < 10) {
        /* Condition using array element that gets modified */
        if (arr[idx] > limit && idx < 8) {
            /* Modify the array element used in condition */
            arr[idx] = limit - 1;  // Modifies arr[idx] used in condition
            
            /* Additional non-trivial operations */
            sum += arr[idx] * idx;
            idx += global_modifier;
            
            /* Another modification to ensure multiple insns in header */
            limit = (limit + 1) % 10;
        } else {
            sum -= arr[idx];
            idx++;
        }
        
        /* Prevent excessive optimization */
        if (side_effect_func() % 3 == 0) {
            limit += 2;
        }
    }
    
    global_counter += sum;
}

/* Test function 4: Nested conditions with modifications */
NOINLINE void test_nested_modification(void) {
    VOLATILE int p = 0;
    VOLATILE int q = 100;
    VOLATILE int r = 50;
    VOLATILE int accumulator = 0;
    
    for (int outer = 0; outer < 20; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            /* Outer condition */
            if (p < q) {
                /* Inner condition that modifies outer condition variable */
                if (r > p && q > 0) {
                    /* This modifies 'p' which is used in outer condition */
                    p = r - inner;  // Modification in inner then block
                    accumulator += p * q;
                    
                    /* Additional modification to create more instructions */
                    q = (q > 200) ? 100 : q + outer;
                } else {
                    r = p + q;
                    accumulator -= r;
                }
            } else {
                q = p - r;
                accumulator += q;
            }
        }
        
        /* Introduce variability */
        p += side_effect_func() % 5;
    }
    
    global_counter += accumulator;
}

/* Test function 5: Modification via function call in then block */
NOINLINE int modify_condition_var(int *var) {
    *var += global_modifier;
    return *var;
}

NOINLINE void test_function_call_modification(void) {
    VOLATILE int base = 100;
    VOLATILE int threshold = 150;
    VOLATILE int total = 0;
    
    for (int cycle = 0; cycle < 30; cycle++) {
        /* Condition with function call */
        if (base < threshold && side_effect_func() > 0) {
            /* Function call modifies 'base' which is in condition */
            modify_condition_var(&base);  // Indirect modification
            
            /* Additional arithmetic */
            total += base * cycle;
            threshold += cycle % 3;
        } else {
            base += threshold;
            total -= base;
        }
        
        /* Ensure loop doesn't become trivial */
        if (cycle % 7 == 0) {
            global_modifier = (global_modifier + 1) % 5;
        }
    }
    
    global_counter += total;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        global_counter = seed;
        global_modifier = (seed % 10) + 1;
    }
    
    printf("Starting tests with seed=%d, global_counter=%d\n", 
           seed, global_counter);
    
    /* Execute all test patterns */
    test_simple_modification();
    printf("After test 1: global_counter=%d\n", global_counter);
    
    test_compound_condition();
    printf("After test 2: global_counter=%d\n", global_counter);
    
    test_pointer_modification();
    printf("After test 3: global_counter=%d\n", global_counter);
    
    test_nested_modification();
    printf("After test 4: global_counter=%d\n", global_counter);
    
    test_function_call_modification();
    printf("After test 5: global_counter=%d\n", global_counter);
    
    /* Final computation to ensure all code is used */
    int final_result = global_counter * global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
