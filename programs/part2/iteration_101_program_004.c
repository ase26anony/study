#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_modifier = 1;

/* Function prototypes with noinline attribute */
__attribute__((noinline)) 
static void test_simple_modification(void);

__attribute__((noinline)) 
static void test_compound_condition(void);

__attribute__((noinline)) 
static void test_loop_with_modification(void);

__attribute__((noinline)) 
static void test_function_call_in_condition(void);

__attribute__((noinline)) 
static int side_effect_func(void);

/* Test 1: Simple modification of condition variable in then block */
__attribute__((noinline)) 
static void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int result = 0;
    
    /* Loop to create basic block context */
    for (int i = 0; i < 100; i++) {
        /* Critical if: modifies 'a' which is used in the condition */
        if (a > b) {           /* test_expr uses 'a' */
            a = b;             /* MODIFICATION: changes 'a' in then block header */
            result += a * i;
        } else {
            result -= b;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        b += (i % 3);
    }
    
    /* Use result to prevent dead code elimination */
    global_counter += result;
}

/* Test 2: Compound condition with multiple modifications */
__attribute__((noinline)) 
static void test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int temp = 0;
    
    /* Multiple iterations to ensure analysis */
    for (int i = 0; i < 50; i++) {
        /* Complex test_expr with && operator */
        if (x != 0 && y < z) {   /* test_expr uses x, y, z */
            x = y;               /* MODIFICATION: changes 'x' used in condition */
            y = z + 1;           /* Also changes 'y' used in condition */
            temp += x * y;
            
            /* Additional non-label, non-note instructions in header */
            z = (z * 2) % 100;
            temp -= z;
        } else {
            x = (x + 1) % 200;
        }
        
        /* Vary condition variables */
        z += (i % 5) - 2;
    }
    
    global_counter += temp;
}

/* Test 3: Loop with counter modification in then block */
__attribute__((noinline)) 
static void test_loop_with_modification(void) {
    static volatile int counter = 0;
    volatile int limit = 1000;
    volatile int accumulator = 0;
    
    /* The loop itself creates the basic block structure */
    while (counter < limit) {
        /* Condition uses counter, modifies it in then block */
        if (counter < (limit / 2)) {   /* test_expr uses 'counter' */
            counter++;                 /* MODIFICATION: changes condition variable */
            accumulator += counter * 2;
            
            /* More instructions in the header */
            limit -= (accumulator % 3);
            global_modifier = counter % 10;
        } else {
            accumulator -= counter;
        }
        
        /* Early exit to prevent infinite loop */
        if (accumulator > 10000) break;
    }
    
    global_counter += accumulator;
}

/* Helper function with side effects */
__attribute__((noinline)) 
static int side_effect_func(void) {
    static int state = 0;
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return (state % 100) - 50;
}

/* Test 4: Function call in condition with modification */
__attribute__((noinline)) 
static void test_function_call_in_condition(void) {
    volatile int a = 0, b = 0, c = 0;
    volatile int res = 0;
    
    for (int i = 0; i < 30; i++) {
        /* Function call in condition - creates complex test_expr */
        if ((a = side_effect_func()) > 0 && b < c) {
            /* Modify variables that might be part of the condition evaluation */
            b = a;                     /* MODIFICATION: changes 'b' used in condition */
            c = side_effect_func();    /* Changes 'c' used in condition */
            res += a + b + c;
            
            /* Additional arithmetic in header */
            a = (a * 3) / 2;           /* Also modifies 'a' */
        } else {
            c = b + 1;
        }
        
        /* Ensure variables change each iteration */
        b += i % 7;
    }
    
    global_counter += res;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    printf("Starting IF-CONVERSION test with %d iterations\n", iterations);
    
    /* Run tests multiple times to increase coverage chances */
    for (int i = 0; i < iterations; i++) {
        test_simple_modification();
        test_compound_condition();
        test_loop_with_modification();
        test_function_call_in_condition();
        
        /* Add some variability between iterations */
        global_modifier = (global_modifier * 13 + 7) % 100;
    }
    
    /* Print results to ensure code isn't eliminated */
    printf("Final counter: %d\n", global_counter);
    printf("Final modifier: %d\n", global_modifier);
    
    return global_counter > 0 ? 0 : 1;
}
