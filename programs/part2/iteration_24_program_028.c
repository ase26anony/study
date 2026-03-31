#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Use argv-based condition to prevent constant folding */
    if (x > 0) {  /* Condition uses x */
        x = 10;    /* MODIFIES condition variable in then block - should fail if-conversion */
        result = y * 2;
        global_acc += result;
    } else {
        result = y / 2;
        global_acc -= result;
    }
    
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((noinline, optimize("O3")))
float test_safe_pattern(float a, float b) {
    float result = 0.0f;
    
    /* Condition variable 'a' is not modified in then block */
    if (a > 0.0f) {
        result = b * 3.14f;  /* Doesn't modify 'a' */
        global_acc += (int)result;
    } else {
        result = b / 3.14f;
        global_acc -= (int)result;
    }
    
    return result;
}

/* Test 3: Pointer dereference condition with modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int local = *ptr;  /* Dereference for condition */
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local > threshold, 1)) {
        *ptr = 0;  /* Modifies memory pointed to by condition variable */
        result = threshold * 2;
        global_acc += result;
    } else {
        result = threshold / 2;
        global_acc -= result;
    }
    
    return result;
}

/* Test 4: Volatile condition variable */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    /* Read volatile for condition */
    if (*v > 100) {
        *v = 50;  /* Modifies volatile condition variable */
        result = 200;
        global_acc += result;
    } else {
        result = 50;
        global_acc -= result;
    }
    
    return result;
}

/* Test 5: Multiple related if-statements in sequence */
__attribute__((noinline, optimize("O2")))
int test_multiple_ifs(int x, int y, int z) {
    int result = 0;
    
    /* First if - unsafe (modifies x in then) */
    if (x > 0) {
        x++;  /* Modifies condition variable */
        result += y;
        global_acc++;
    } else {
        result -= y;
        global_acc--;
    }
    
    /* Second if - safe (doesn't modify y) */
    if (y < 10) {
        result += z * 2;
        global_acc += 2;
    } else {
        result += z / 2;
        global_acc += 3;
    }
    
    /* Third if - modifies different variable */
    if (z == 0) {
        y = 5;  /* Modifies y, but z is condition variable */
        result *= 2;
        global_acc *= 2;
    }
    
    return result;
}

/* Test 6: Complex expression in condition */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition using multiple variables */
    if ((a * b) > (c + 10)) {
        a = b + c;  /* Modifies 'a' which is part of condition */
        result = a * 2;
        global_acc += result;
    } else {
        result = c * 3;
        global_acc -= result;
    }
    
    return result;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O2")))
int test_loop_with_if(int *arr, int size, int limit) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* This if inside loop is a candidate for if-conversion */
        if (arr[i] > limit) {
            limit = arr[i] / 2;  /* Modifies condition variable in then block */
            sum += arr[i];
        } else {
            sum -= arr[i];
        }
    }
    
    global_acc += sum;
    return sum;
}

/* Test 8: Function call that might modify condition */
__attribute__((noinline))
int helper_modify(int *x) {
    *x = *x + 1;
    return *x;
}

__attribute__((optimize("O3")))
int test_function_call_condition(int x, int y) {
    int result = 0;
    
    if (x > 5) {
        /* Function call modifies x through pointer */
        helper_modify(&x);  /* Indirect modification of condition variable */
        result = y * 10;
        global_acc += result;
    } else {
        result = y * 5;
        global_acc -= result;
    }
    
    return result;
}

/* Test 9: Mixed types and operations */
__attribute__((noinline, optimize("O2")))
float test_mixed_types(int x, float y) {
    float result = 0.0f;
    
    if (x > 0) {
        x = x * 2;  /* Modifies int condition variable */
        result = y * 2.0f;
        global_acc += (int)result;
    } else {
        result = y / 2.0f;
        global_acc -= (int)result;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i;
    int seed;
    
    /* Use argv for runtime variability to prevent constant folding */
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
    float a = (float)(rand() % 100) / 10.0f;
    float b = (float)(rand() % 100) / 10.0f;
    
    volatile int volatile_var = rand() % 200;
    int ptr_val = rand() % 150;
    int *ptr = &ptr_val;
    
    /* Array for loop test */
    int arr[10];
    for (i = 0; i < 10; i++) {
        arr[i] = rand() % 100;
    }
    
    printf("Starting tests with seed: %d\n", seed);
    printf("Initial global_acc: %d\n", global_acc);
    
    /* Execute all test functions */
    int r1 = test_unsafe_modification(x, y);
    printf("Test1 result: %d, global_acc: %d\n", r1, global_acc);
    
    float r2 = test_safe_pattern(a, b);
    printf("Test2 result: %.2f, global_acc: %d\n", r2, global_acc);
    
    int r3 = test_pointer_condition(ptr, 50);
    printf("Test3 result: %d, global_acc: %d\n", r3, global_acc);
    
    int r4 = test_volatile_condition(&volatile_var);
    printf("Test4 result: %d, global_acc: %d\n", r4, global_acc);
    
    int r5 = test_multiple_ifs(x, y, z);
    printf("Test5 result: %d, global_acc: %d\n", r5, global_acc);
    
    int r6 = test_complex_condition(x, y, z);
    printf("Test6 result: %d, global_acc: %d\n", r6, global_acc);
    
    int r7 = test_loop_with_if(arr, 10, 50);
    printf("Test7 result: %d, global_acc: %d\n", r7, global_acc);
    
    int r8 = test_function_call_condition(x, y);
    printf("Test8 result: %d, global_acc: %d\n", r8, global_acc);
    
    float r9 = test_mixed_types(x, b);
    printf("Test9 result: %.2f, global_acc: %d\n", r9, global_acc);
    
    printf("Final global_acc: %d\n", global_acc);
    
    return 0;
}
