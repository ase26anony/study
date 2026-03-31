#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then block */
    if (x > 0) {  /* test_expr = x > 0 */
        x = y * 2;  /* MODIFIES condition variable x */
        result = x + 10;
    } else {
        result = x - 5;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int result = 0;
    
    /* Condition variable a is NOT modified in then block */
    if (a > b) {
        result = a * 2 + b;
    } else {
        result = b * 3 - a;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    /* Condition uses pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold - 1;  /* MODIFIES the dereferenced value */
        result = *ptr * 2;
    } else {
        result = threshold + *ptr;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float g) {
    float result = 0.0f;
    
    if (f > g) {
        f = g * 1.5f;  /* MODIFIES condition variable f */
        result = f + 10.0f;
    } else {
        result = g - f;
    }
    
    global_accumulator += (int)result;
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    if (*v > 100) {
        *v = 50;  /* MODIFIES volatile condition */
        result = *v * 3;
    } else {
        result = *v + 20;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 6: Complex expression in condition with modification */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition expression */
    if ((a * b) > (c + 10)) {
        a = b + c;  /* MODIFIES part of condition expression (a) */
        result = a * 2;
    } else {
        result = c * 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 7: Multiple if-statements in sequence */
__attribute__((noinline, optimize("O2")))
int test_multiple_ifs(int x, int y, int z) {
    int result = 0;
    
    /* First if - safe */
    if (x > y) {
        result += x - y;
    } else {
        result += y - x;
    }
    
    /* Second if - unsafe (modifies y in then block) */
    if (y < z) {
        y = z + 1;  /* MODIFIES condition variable y */
        result += y * 2;
    } else {
        result += z * 3;
    }
    
    /* Third if - uses __builtin_expect */
    if (__builtin_expect(x > 0, 1)) {
        result += 100;
    } else {
        result += 50;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 8: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O3")))
int test_loop_with_if(int n, int *data) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* This if might be if-converted */
        if (data[i] > 0) {
            sum += data[i] * 2;
        } else {
            sum -= data[i];
        }
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 9: Nested if with modification in inner block */
__attribute__((noinline, optimize("O2")))
int test_nested_if(int a, int b, int c) {
    int result = 0;
    
    if (a > 0) {
        if (b > c) {
            a = b + c;  /* MODIFIES outer condition variable a */
            result = a * 3;
        } else {
            result = c * 2;
        }
    } else {
        result = a + b + c;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 10: Function call that might modify condition */
extern int external_func(int *);
__attribute__((noinline, optimize("O3")))
int test_function_call_condition(int x, int y) {
    int result = 0;
    
    if (x > y) {
        /* Function call might modify x through pointer */
        external_func(&x);  /* POTENTIALLY modifies condition variable */
        result = x + y;
    } else {
        result = y - x;
    }
    
    global_accumulator += result;
    return result;
}

/* Dummy external function */
int external_func(int *p) {
    *p += 1;
    return *p;
}

int main(int argc, char *argv[]) {
    int seed;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test variables with random values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 100) / 10.0f;
    float g = (float)(rand() % 100) / 10.0f;
    volatile int volatile_var = rand() % 200;
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = (rand() % 100) - 50;
    }
    
    /* Call all test functions to exercise different patterns */
    int result1 = test_unsafe_modification(x, y);
    int result2 = test_safe_pattern(x, y);
    int result3 = test_pointer_condition(&x, 50);
    float result4 = test_float_condition(f, g);
    int result5 = test_volatile_condition(&volatile_var);
    int result6 = test_complex_condition(x, y, z);
    int result7 = test_multiple_ifs(x, y, z);
    int result8 = test_loop_with_if(10, data);
    int result9 = test_nested_if(x, y, z);
    int result10 = test_function_call_condition(x, y);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d %.2f %d %d %d %d %d %d\n",
           result1, result2, result3, result4, result5,
           result6, result7, result8, result9, result10);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Seed: %d\n", seed);
    
    return 0;
}
