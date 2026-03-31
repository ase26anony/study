#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization and create side effects */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Test 1: Unsafe modification - condition variable modified in then block */
__attribute__((noinline, optimize("O3")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable */
        result = y * 2;
        global_counter++;
    } else {
        result = y / 2;
    }
    
    /* Additional computation to prevent dead code elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O2")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    
    /* Condition variable x is NOT modified in then block */
    if (x > 0) {
        result = y * 3;
        global_counter += 2;
    } else {
        result = y / 3;
    }
    
    /* Use x in computation but don't modify it in then block */
    return result + (x % 10);
}

/* Test 3: Pointer dereference condition with unsafe modification */
__attribute__((noinline, optimize("O3")))
int test_pointer_condition(int *ptr, int y) {
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > 100) {
        *ptr = 50;  /* MODIFIES condition expression */
        result = y + 100;
        global_counter += 3;
    } else {
        result = y - 100;
    }
    
    return result;
}

/* Test 4: Float condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
float test_float_condition(float f, int y) {
    float result = 0.0f;
    
    if (f > 0.5f) {
        f = 1.0f;  /* MODIFIES condition variable */
        result = y * 1.5f;
        global_counter += 4;
    } else {
        result = y * 0.5f;
    }
    
    return result + f;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int *v, int y) {
    int result = 0;
    
    if (*v > 10) {
        *v = 5;  /* MODIFIES volatile condition variable */
        result = y << 2;
        global_counter += 5;
    } else {
        result = y >> 2;
    }
    
    return result;
}

/* Test 6: Complex expression in condition with partial modification */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition expression */
    if ((a * b) > (c + 10)) {
        a = b + c;  /* Modifies part of condition expression (a) */
        result = a * 2;
        global_counter += 6;
    } else {
        result = b * 3;
    }
    
    return result;
}

/* Test 7: Loop with if-conversion candidate (safe) */
__attribute__((noinline, optimize("O3")))
int test_loop_safe(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int temp = i * 2;
        
        /* Safe for if-conversion - condition variable not modified */
        if (temp > iterations) {
            sum += temp * 3;
            global_counter++;
        } else {
            sum += temp;
        }
    }
    
    return sum;
}

/* Test 8: Loop with if-conversion candidate (unsafe) */
__attribute__((noinline, optimize("O2")))
int test_loop_unsafe(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int temp = i * 2;
        
        /* Unsafe for if-conversion - modifies condition variable */
        if (temp > iterations) {
            temp = iterations;  /* MODIFIES condition variable */
            sum += temp * 3;
            global_counter += 2;
        } else {
            sum += temp;
        }
    }
    
    return sum;
}

/* Test 9: Nested if statements with mixed safety */
__attribute__((noinline, optimize("O3")))
int test_nested_ifs(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        /* Outer then block - safe */
        if (y > 0) {
            x = y + z;  /* Modifies outer condition variable */
            result = x * y;
            global_counter += 7;
        } else {
            result = y * z;
        }
    } else {
        if (z > 0) {
            result = x * z;
        } else {
            result = x + y + z;
        }
    }
    
    return result;
}

/* Test 10: Using __builtin_expect to influence branch prediction */
__attribute__((noinline, optimize("O2")))
int test_builtin_expect(int x, int y) {
    int result = 0;
    
    /* Hint that x > 0 is likely */
    if (__builtin_expect(x > 0, 1)) {
        x = y * 2;  /* MODIFIES condition variable */
        result = x + 10;
        global_counter += 8;
    } else {
        result = y - 10;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, result = 0;
    int seed;
    
    /* Use command line argument or time for randomness */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test variables with non-constant values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 100) / 100.0f;
    volatile int volatile_var = rand() % 50;
    int array[10];
    
    for (i = 0; i < 10; i++) {
        array[i] = rand() % 100;
    }
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    
    /* Execute all test functions to trigger if-conversion analysis */
    result += test_unsafe_modification(x, y);
    result += test_safe_pattern(y, z);
    result += test_pointer_condition(&array[0], x);
    result += (int)test_float_condition(f, y);
    result += test_volatile_condition(&volatile_var, z);
    result += test_complex_condition(x, y, z);
    result += test_loop_safe(10 + (rand() % 20));
    result += test_loop_unsafe(10 + (rand() % 20));
    result += test_nested_ifs(x, y, z);
    result += test_builtin_expect(z, x);
    
    /* Use results to prevent dead code elimination */
    global_result = result;
    printf("Final result: %d (global_counter: %d)\n", result, global_counter);
    
    return (global_result > 0) ? 0 : 1;
}
