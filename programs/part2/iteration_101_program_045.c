#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Global variables to create dependencies */
VOLATILE int global_counter = 0;
VOLATILE int global_modifier = 1;

/* Function with side effects for condition */
NOINLINE int side_effect_func(void) {
    return global_counter++;
}

/* Test 1: Simple modification in then block */
NOINLINE void test1_modify_condition_var(void) {
    VOLATILE int a = 10, b = 5, c = 0;
    
    /* Loop context to create interesting block structure */
    for (int i = 0; i < 100; i++) {
        /* Condition with variable used in then block */
        if (a > b) {
            /* EARLY modification of condition variable in then block header */
            a = b + i;  // This modifies 'a' used in condition
            c += a * 2;
        }
        /* Prevent loop optimization */
        b += (i & 1);
    }
    
    /* Use results to prevent elimination */
    printf("Test1: a=%d, b=%d, c=%d\n", a, b, c);
}

/* Test 2: Compound condition with multiple modifications */
NOINLINE void test2_compound_condition(void) {
    VOLATILE int x = 100, y = 50, z = 75;
    VOLATILE int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Complex compound condition */
        if (x != 0 && y < z && (x + y) > (z * 2)) {
            /* Multiple modifications of condition variables */
            x = y + 1;      // Modifies 'x' used in condition
            y = z - i;      // Modifies 'y' used in condition  
            z = x * 2;      // Modifies 'z' used in condition
            result += x + y + z;
        }
        /* Create data flow to prevent dead code elimination */
        x += (i % 3);
        y -= (i % 2);
        z += (i % 5);
    }
    
    printf("Test2: x=%d, y=%d, z=%d, result=%d\n", x, y, z, result);
}

/* Test 3: Static variable modified in then block */
NOINLINE void test3_static_modification(void) {
    static VOLATILE int counter = 0;
    VOLATILE int limit = 1000;
    VOLATILE int accumulator = 0;
    
    for (int i = 0; i < 200; i++) {
        /* Condition using static variable */
        if (counter < limit) {
            /* Modification of condition variable */
            counter++;  // This modifies 'counter' used in condition
            accumulator += counter;
            
            /* Additional non-trivial computation */
            if (accumulator > 5000) {
                accumulator = 0;
            }
        }
        /* Vary the limit */
        limit -= (i % 10);
    }
    
    printf("Test3: counter=%d, accumulator=%d\n", counter, accumulator);
}

/* Test 4: Function call in condition with side effects */
NOINLINE void test4_function_in_condition(void) {
    VOLATILE int a = 0, b = 10;
    VOLATILE int total = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Function call with side effects in condition */
        if (side_effect_func() > (i / 2)) {
            /* Modify global variable that function reads */
            global_counter += 2;  // Modifies variable used by side_effect_func()
            a = b * i;
            b = a - global_counter;
            total += a + b;
        }
        /* Ensure loop has side effects */
        global_modifier *= (i + 1);
        if (global_modifier > 1000000) global_modifier = 1;
    }
    
    printf("Test4: a=%d, b=%d, total=%d, global_counter=%d\n", 
           a, b, total, global_counter);
}

/* Test 5: Nested conditions with modifications */
NOINLINE void test5_nested_modifications(void) {
    VOLATILE int p = 100, q = 200, r = 150;
    VOLATILE int output = 0;
    
    for (int outer = 0; outer < 20; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            /* Complex nested condition structure */
            if (p > q || (q < r && p != 0)) {
                /* Multiple instructions in then block header */
                p = q - inner;  // Modifies 'p' used in condition
                output += p;
                
                if (r > 100) {
                    q = r + outer;  // Modifies 'q' used in condition
                    output += q;
                }
                
                r = p + q;  // Modifies 'r' used in condition
                output += r;
            }
        }
        /* Prevent optimization */
        p += (outer % 7);
        q -= (outer % 3);
        r += (outer % 5);
    }
    
    printf("Test5: p=%d, q=%d, r=%d, output=%d\n", p, q, r, output);
}

/* Main function with runtime variability */
int main(int argc, char *argv[]) {
    /* Use command line arguments for runtime variability */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with some randomness */
    srand(seed);
    global_counter = rand() % 100;
    
    printf("Starting tests with seed=%d, global_counter=%d\n", seed, global_counter);
    
    /* Run all test patterns */
    test1_modify_condition_var();
    test2_compound_condition();
    test3_static_modification();
    test4_function_in_condition();
    test5_nested_modifications();
    
    /* Final computation with observable result */
    int final_result = global_counter + global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result % 100;
}
