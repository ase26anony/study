#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable - should fail if-conversion */
        result = y * 2;
        global_acc += result;
    } else {
        result = y + 5;
        global_acc += result;
    }
    
    /* Additional code to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        result += i;
    }
    
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int x, int y) {
    int result = 0;
    int local_x = x;  /* Copy to ensure original isn't modified */
    
    /* Condition variable local_x is NOT modified in the then block */
    if (local_x > 0) {
        result = y * 3;
        global_acc += result;
    } else {
        result = y - 2;
        global_acc += result;
    }
    
    /* Mix with other operations */
    result = __builtin_expect(result > 0, 1) ? result : -result;
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int y) {
    int result = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > 100) {
        *ptr = 50;  /* MODIFIES the dereferenced value - should fail if-conversion */
        result = y * 4;
        global_acc += result;
    } else {
        result = y / 2;
        global_acc += result;
    }
    
    return result;
}

/* Test 4: Float condition with unsafe modification */
__attribute__((optimize("O3"), noinline))
float test_float_condition(float x, float y) {
    float result = 0.0f;
    
    /* Float condition variable */
    if (x > 0.5f) {
        x = 0.0f;  /* MODIFIES condition variable - should fail if-conversion */
        result = y * 1.5f;
        global_acc += (int)result;
    } else {
        result = y + 2.5f;
        global_acc += (int)result;
    }
    
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((optimize("O2"), noinline))
int test_volatile_condition(volatile int *v, int y) {
    int result = 0;
    
    /* Volatile access in condition */
    if (*v > 0) {
        *v = -1;  /* MODIFIES volatile - should fail if-conversion */
        result = y * 5;
        global_acc += result;
    } else {
        result = y * 3;
        global_acc += result;
    }
    
    return result;
}

/* Test 6: Multiple related if-statements in sequence */
__attribute__((optimize("O3"), noinline))
int test_multiple_ifs(int x, int y, int z) {
    int result = 0;
    
    /* First if - safe */
    if (x > 0) {
        result += y;
        global_acc += 1;
    } else {
        result -= y;
        global_acc += 2;
    }
    
    /* Second if - unsafe (modifies y which might be used elsewhere) */
    if (y < 10) {
        y = 20;  /* MODIFIES variable that might be condition elsewhere */
        result += z * 2;
        global_acc += 3;
    } else {
        result += z;
        global_acc += 4;
    }
    
    /* Third if - depends on modified y */
    if (y > 15) {
        result *= 2;
        global_acc += 5;
    }
    
    return result;
}

/* Test 7: Complex expression in condition */
__attribute__((optimize("O2"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    int cond_var = a + b;
    
    /* Complex condition expression */
    if ((cond_var * c) > (a * b)) {
        cond_var = 0;  /* MODIFIES part of condition expression */
        result = a + b + c;
        global_acc += result;
    } else {
        result = a * b * c;
        global_acc += result;
    }
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 2; i++) {
        result += i * cond_var;
    }
    
    return result;
}

/* Test 8: Function call in then block that might modify condition */
extern int external_func(int*);
__attribute__((optimize("O3"), noinline))
int test_function_call(int x, int y) {
    int result = 0;
    
    if (x > 0) {
        /* Function call that takes address of x - might modify it */
        result = external_func(&x);  /* POTENTIALLY modifies condition variable */
        global_acc += result;
    } else {
        result = y * 2;
        global_acc += result;
    }
    
    return result;
}

/* Dummy external function */
int external_func(int *p) {
    *p = *p + 1;  /* Actually modifies the parameter */
    return *p * 2;
}

/* Test 9: Nested if with modification in inner block */
__attribute__((optimize("O2"), noinline))
int test_nested_if(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            x = y;  /* MODIFIES outer condition variable in inner block */
            result = z * 3;
            global_acc += result;
        } else {
            result = z * 2;
            global_acc += result;
        }
    } else {
        result = z;
        global_acc += result;
    }
    
    return result;
}

/* Test 10: Array element as condition variable */
__attribute__((optimize("O3"), noinline))
int test_array_condition(int arr[4], int idx) {
    int result = 0;
    
    /* Condition uses array element */
    if (arr[idx] > 10) {
        arr[idx] = 0;  /* MODIFIES the array element used in condition */
        result = idx * 10;
        global_acc += result;
    } else {
        result = idx * 5;
        global_acc += result;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    int cond_var2 = rand() % 100;
    float cond_var3 = (float)(rand() % 100) / 100.0f;
    volatile int cond_var4 = rand() % 100;
    int array[4] = {rand() % 20, rand() % 20, rand() % 20, rand() % 20};
    int ptr_val = rand() % 200;
    
    int total_result = 0;
    
    /* Call test functions with runtime-determined values */
    total_result += test_unsafe_modification(cond_var1, cond_var2);
    total_result += test_safe_pattern(cond_var2, cond_var1);
    total_result += test_pointer_condition(&ptr_val, cond_var1);
    total_result += (int)test_float_condition(cond_var3, (float)cond_var2);
    
    volatile int volatile_var = cond_var4;
    total_result += test_volatile_condition(&volatile_var, cond_var1);
    
    total_result += test_multiple_ifs(cond_var1, cond_var2, rand() % 50);
    total_result += test_complex_condition(cond_var1, cond_var2, rand() % 10);
    total_result += test_function_call(cond_var1, cond_var2);
    total_result += test_nested_if(cond_var1, cond_var2, rand() % 30);
    total_result += test_array_condition(array, rand() % 4);
    
    /* Print results to prevent optimization */
    printf("Seed: %d\n", seed);
    printf("Total result: %d\n", total_result);
    printf("Global accumulator: %d\n", global_acc);
    
    return total_result > 0 ? 0 : 1;
}
