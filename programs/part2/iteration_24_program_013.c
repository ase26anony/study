#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* This should trigger the safety check - x is modified in then block */
    if (x > 0) {
        x = y * 2;  /* MODIFIES condition variable x */
        result = 100;
    } else {
        result = 200;
    }
    
    global_counter++;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    
    /* This should pass the safety check - x is not modified in then block */
    if (x > y) {
        result = x + y;  /* Doesn't modify x */
    } else {
        result = x - y;
    }
    
    global_counter += 2;
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    /* Condition uses pointer dereference, modified in then block */
    if (*ptr > threshold) {
        *ptr = threshold;  /* MODIFIES dereferenced condition variable */
        result = 300;
    } else {
        result = 400;
    }
    
    global_counter += 3;
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float inc) {
    float result = 0.0f;
    
    /* Float condition variable modified in then block */
    if (f > 0.5f) {
        f += inc;  /* MODIFIES condition variable f */
        result = f * 2.0f;
    } else {
        result = f / 2.0f;
    }
    
    global_counter += 4;
    return result;
}

/* Test 5: Volatile variable condition */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    /* Volatile access in condition */
    if (*v > 10) {
        *v = 5;  /* MODIFIES volatile condition variable */
        result = 500;
    } else {
        result = 600;
    }
    
    global_counter += 5;
    return result;
}

/* Test 6: Complex expression in condition with partial modification */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition using multiple variables */
    if ((a + b) > c) {
        a = b + c;  /* MODIFIES part of condition expression (a) */
        result = 700;
    } else {
        result = 800;
    }
    
    global_counter += 6;
    return result;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O2")))
int test_loop_with_branch(int n, int *data) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch inside loop - good candidate for if-conversion */
        if (data[i] > 0) {
            data[i] = -data[i];  /* MODIFIES condition variable data[i] */
            sum += 1;
        } else {
            sum += 2;
        }
    }
    
    global_counter += 7;
    return sum;
}

/* Test 8: Multiple related if-statements */
__attribute__((noinline, optimize("O3")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First branch - safe */
    if (x > 0) {
        result += 10;
    } else {
        result += 20;
    }
    
    /* Second branch - unsafe (modifies y used in next condition) */
    if (y > 0) {
        y = z * 2;  /* MODIFIES condition variable for next branch */
        result += 30;
    } else {
        result += 40;
    }
    
    /* Third branch - uses modified y */
    if (y > z) {
        result += 50;
    } else {
        result += 60;
    }
    
    global_counter += 8;
    return result;
}

/* Test 9: Using __builtin_expect to influence branch prediction */
__attribute__((noinline, optimize("O2")))
int test_builtin_expect(int x, int y) {
    int result = 0;
    
    /* Hint that x > y is likely */
    if (__builtin_expect(x > y, 1)) {
        x = y + 1;  /* MODIFIES condition variable */
        result = 900;
    } else {
        result = 1000;
    }
    
    global_counter += 9;
    return result;
}

/* Test 10: Nested if statements */
__attribute__((noinline, optimize("O3")))
int test_nested_branches(int a, int b, int c) {
    int result = 0;
    
    if (a > 0) {
        if (b > 0) {
            a = c;  /* MODIFIES outer condition variable */
            result = 1100;
        } else {
            result = 1200;
        }
    } else {
        result = 1300;
    }
    
    global_counter += 10;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line argument or time for randomness */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test variables */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)rand() / RAND_MAX;
    int data[10];
    volatile int volatile_var = rand() % 20;
    
    for (int i = 0; i < 10; i++) {
        data[i] = rand() % 100 - 50;
    }
    
    /* Execute all test functions */
    int result1 = test_unsafe_modification(x, y);
    int result2 = test_safe_pattern(x, y);
    
    int ptr_val = rand() % 100;
    int result3 = test_pointer_condition(&ptr_val, 50);
    
    float result4 = test_float_condition(f, 0.1f);
    int result5 = test_volatile_condition(&volatile_var);
    int result6 = test_complex_condition(x, y, z);
    int result7 = test_loop_with_branch(10, data);
    int result8 = test_multiple_branches(x, y, z);
    int result9 = test_builtin_expect(x, y);
    int result10 = test_nested_branches(x, y, z);
    
    /* Aggregate results to prevent optimization */
    global_result = result1 + result2 + result3 + (int)result4 + 
                    result5 + result6 + result7 + result8 + result9 + result10;
    
    printf("Seed: %d\n", seed);
    printf("Global counter: %d\n", global_counter);
    printf("Global result: %d\n", global_result);
    
    /* Use results to affect control flow */
    if (global_result > 10000) {
        printf("Large result\n");
    } else {
        printf("Small result\n");
    }
    
    return 0;
}
