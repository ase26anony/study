#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimizations */
volatile int global_counter = 0;
int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Force runtime evaluation */
    if (global_counter++ % 3 == 0) {
        x = rand() % 100;
    }
    
    /* Critical if-statement: condition variable x modified in then-block */
    if (x > 50) {
        /* UNSAFE: modifies condition variable x */
        x = x + 10;  /* This should trigger the safety check */
        result = y * 2;
        global_accumulator += result;
    } else {
        result = y / 2;
        global_accumulator -= result;
    }
    
    /* Use result to prevent elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
float test_safe_pattern(float a, float b) {
    float result = 0.0f;
    volatile float cond_var = a;
    
    /* Multiple if-statements to create larger basic block */
    if (cond_var > 0.0f) {
        result = b * 3.14f;
        global_accumulator += (int)result;
    } else {
        result = b / 3.14f;
        global_accumulator -= (int)result;
    }
    
    /* Another if-statement with hint */
    if (__builtin_expect(cond_var < 100.0f, 1)) {
        result += 1.0f;
    }
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int local_copy = *ptr;
    int result = 0;
    
    /* Complex condition with pointer dereference */
    if (local_copy > threshold && *ptr != 0) {
        /* UNSAFE: modifies through pointer */
        *ptr = local_copy + 5;  /* Modifies memory used in condition */
        result = threshold * 3;
        global_accumulator ^= result;
    } else {
        result = threshold / 3;
        global_accumulator |= result;
    }
    
    return result;
}

/* Test 4: Mixed types with volatile condition */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int *vptr) {
    int result = 0;
    int temp = *vptr;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        if (temp > global_cond) {
            /* UNSAFE: modifies volatile location */
            *vptr = temp - 1;  /* Modifies volatile used in condition */
            result += i * 10;
        } else {
            result -= i * 5;
        }
        temp = *vptr;  /* Re-read volatile */
    }
    
    return result;
}

/* Test 5: Safe pattern with multiple condition variables */
__attribute__((noinline, optimize("O2")))
int test_safe_multiple(int a, int b, int c) {
    int result = 0;
    
    /* Multiple related conditions */
    if (a > b) {
        result = c * a;
        /* SAFE: modifies different variable */
        b = b + 1;
    } else {
        result = c * b;
        a = a + 1;
    }
    
    /* Second if-statement */
    if (result < 1000) {
        result += global_counter;
    }
    
    return result;
}

/* Test 6: Unsafe with increment operation */
__attribute__((noinline, optimize("O3")))
int test_unsafe_increment(int base) {
    int counter = base;
    int result = 0;
    
    /* Loop with if inside */
    for (int i = 0; i < 4; i++) {
        if (counter++ < 10) {  /* Condition uses counter */
            /* UNSAFE: modifies counter in then-block */
            counter += 2;  /* This modification should be detected */
            result += i * 20;
        } else {
            result -= i * 10;
        }
        
        /* Prevent loop unrolling */
        if (global_counter % 2 == 0) {
            counter += rand() % 3;
        }
    }
    
    return result;
}

/* Test 7: Function call that might modify condition */
extern int external_func(int*);
int side_effect_var = 0;

__attribute__((noinline, optimize("O2")))
int test_function_call_condition(int val) {
    int condition_var = val;
    int result = 0;
    
    if (condition_var > 100) {
        /* Potentially unsafe: function call might modify condition_var */
        result = external_func(&condition_var);
        global_accumulator += result;
    } else {
        result = condition_var * 2;
        global_accumulator -= result;
    }
    
    return result;
}

/* Dummy external function */
int external_func(int *p) {
    *p += 5;  /* Modifies the parameter */
    side_effect_var++;
    return *p * 2;
}

/* Main driver with runtime-controlled paths */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for runtime control */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize test variables */
    int x = rand() % 200;
    int y = rand() % 100;
    float a = (float)(rand() % 1000) / 10.0f;
    float b = (float)(rand() % 500) / 5.0f;
    int ptr_val = rand() % 200;
    int *ptr = &ptr_val;
    volatile int volatile_val = rand() % 150;
    volatile int *vptr = &volatile_val;
    
    int total = 0;
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    printf("Initial values: x=%d, y=%d, a=%.2f, b=%.2f\n", x, y, a, b);
    
    /* Execute all test functions */
    total += test_unsafe_modification(x, y);
    printf("After test1: accumulator=%d\n", global_accumulator);
    
    total += (int)test_safe_pattern(a, b);
    printf("After test2: accumulator=%d\n", global_accumulator);
    
    total += test_pointer_condition(ptr, 75);
    printf("After test3: ptr_val=%d, accumulator=%d\n", ptr_val, global_accumulator);
    
    total += test_volatile_condition(vptr);
    printf("After test4: volatile_val=%d, accumulator=%d\n", volatile_val, global_accumulator);
    
    total += test_safe_multiple(x, y, 10);
    printf("After test5: accumulator=%d\n", global_accumulator);
    
    total += test_unsafe_increment(5);
    printf("After test6: accumulator=%d\n", global_accumulator);
    
    total += test_function_call_condition(150);
    printf("After test7: side_effect_var=%d, accumulator=%d\n", side_effect_var, global_accumulator);
    
    printf("Final total: %d\n", total);
    printf("Final accumulator: %d\n", global_accumulator);
    
    return total > 0 ? 0 : 1;
}
