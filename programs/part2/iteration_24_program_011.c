#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Test 1: Unsafe modification of integer condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_int_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then-block */
    if (x > 0) {  /* test_expr: x > 0 */
        x = y * 2;  /* MODIFIES condition variable x */
        result = x + 10;
        global_acc += result;
    } else {
        result = x - 5;
        global_acc += result;
    }
    
    /* Additional computation to encourage if-conversion */
    result = (result > 0) ? result * 2 : result / 2;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O3")))
int test_safe_int_pattern(int x, int y) {
    int result = 0;
    
    /* Condition variable x is NOT modified in then-block */
    if (x > 0) {  /* test_expr: x > 0 */
        result = y * 2;  /* Does NOT modify x */
        global_acc += result;
    } else {
        result = x - 5;
        global_acc += result;
    }
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(result > 100, 0)) {
        result /= 2;
    }
    
    return result;
}

/* Test 3: Pointer dereference modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > threshold) {  /* test_expr: *ptr > threshold */
        *ptr = threshold - 1;  /* MODIFIES the dereferenced value */
        result = *ptr * 3;
        global_acc += result;
    } else {
        result = threshold + *ptr;
        global_acc += result;
    }
    
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_modification(float a, float b) {
    float result = 0.0f;
    
    /* Float condition variable */
    if (a > 0.0f) {  /* test_expr: a > 0.0f */
        a = b * 2.0f;  /* MODIFIES condition variable a */
        result = a + 1.5f;
        global_acc += (int)result;
    } else {
        result = a - 1.5f;
        global_acc += (int)result;
    }
    
    /* Multiple related if-statements */
    if (result > 10.0f) {
        result /= 2.0f;
    }
    
    return result;
}

/* Test 5: Volatile variable modification */
__attribute__((noinline, optimize("O2")))
int test_volatile_modification(volatile int *v) {
    int result = 0;
    
    /* Condition using volatile variable */
    if (*v > 0) {  /* test_expr: *v > 0 */
        (*v)++;  /* MODIFIES the volatile variable */
        result = *v * 2;
        global_acc += result;
    } else {
        result = *v - 2;
        global_acc += result;
    }
    
    return result;
}

/* Test 6: Complex expression modification */
__attribute__((noinline, optimize("O3")))
int test_complex_expression(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition expression */
    if ((a + b) > c) {  /* test_expr: (a + b) > c */
        a = b + c;  /* MODIFIES part of condition expression (a) */
        result = a * 3;
        global_acc += result;
    } else {
        result = c - a;
        global_acc += result;
    }
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        result += i;
    }
    
    return result;
}

/* Test 7: Global variable modification */
__attribute__((noinline, optimize("O2")))
int test_global_modification(int x) {
    int result = 0;
    
    /* Condition using global variable */
    if (global_acc > x) {  /* test_expr: global_acc > x */
        global_acc = x;  /* MODIFIES the global variable used in condition */
        result = global_acc * 2;
    } else {
        result = x - global_acc;
    }
    
    return result;
}

/* Test 8: Safe pattern with multiple branches */
__attribute__((noinline, optimize("O3")))
int test_multi_branch_safe(int x, int y, int z) {
    int result = 0;
    
    /* Multiple if-statements in sequence */
    if (x > 0) {  /* test_expr: x > 0 */
        result = y + z;  /* Does NOT modify x */
    } else {
        result = y - z;
    }
    
    if (y > 0) {  /* Separate condition */
        result *= 2;
    } else {
        result /= 2;
    }
    
    global_acc += result;
    return result;
}

/* Test 9: Array element modification */
__attribute__((noinline, optimize("O2")))
int test_array_modification(int arr[], int idx) {
    int result = 0;
    
    /* Condition based on array element */
    if (arr[idx] > 0) {  /* test_expr: arr[idx] > 0 */
        arr[idx] = 0;  /* MODIFIES the array element used in condition */
        result = idx * 2;
        global_acc += result;
    } else {
        result = idx + 1;
        global_acc += result;
    }
    
    return result;
}

/* Test 10: Mixed safe and unsafe in same function */
__attribute__((noinline, optimize("O3")))
int test_mixed_patterns(int a, int b, int c) {
    int result = 0;
    
    /* UNSAFE: modifies condition variable */
    if (a > b) {
        a = c;  /* MODIFIES condition variable a */
        result += a * 2;
    } else {
        result += b * 2;
    }
    
    /* SAFE: does not modify condition variable */
    if (b > c) {
        result += 10;  /* Does NOT modify b */
    } else {
        result += 20;
    }
    
    global_acc += result;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv to create runtime-dependent values */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = rand();
    }
    
    srand(seed);
    
    /* Initialize test variables with runtime-dependent values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    
    volatile int volatile_var = rand() % 50;
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = rand() % 100 - 50;
    }
    
    int *ptr = &x;
    
    /* Execute all test functions */
    int results[10];
    
    results[0] = test_unsafe_int_modification(x, y);
    results[1] = test_safe_int_pattern(x + 1, y);
    results[2] = test_pointer_condition(ptr, 50);
    results[3] = (int)test_float_modification(f1, f2);
    results[4] = test_volatile_modification(&volatile_var);
    results[5] = test_complex_expression(x, y, z);
    results[6] = test_global_modification(z);
    results[7] = test_multi_branch_safe(x, y, z);
    results[8] = test_array_modification(array, 3);
    results[9] = test_mixed_patterns(x, y, z);
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += results[i];
    }
    
    printf("Final result: %d (Global accumulator: %d)\n", final_result, global_acc);
    printf("Seed used: %d\n", seed);
    
    return final_result != 0 ? 0 : 1;
}
