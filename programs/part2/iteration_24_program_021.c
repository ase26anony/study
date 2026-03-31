#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - condition variable modified in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then-block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable */
        result = y * 2;
        global_accumulator += result;
    } else {
        result = y / 2;
        global_accumulator -= result;
    }
    
    /* Use x to prevent dead code elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int local = a;
    
    /* Condition variable a is NOT modified in then-block */
    if (a > 100) {
        local = b * 3;  /* Modifies local, not a */
        global_accumulator += local;
    } else {
        local = b / 3;
        global_accumulator -= local;
    }
    
    return local + a;  /* a remains unchanged */
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int val = *ptr;
    int result = 0;
    
    /* Condition depends on pointer dereference */
    if (val > threshold) {
        *ptr = threshold;  /* MODIFIES the memory condition depends on */
        result = val - threshold;
        global_accumulator += result;
    } else {
        result = threshold - val;
        global_accumulator -= result;
    }
    
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float g) {
    float result = 0.0f;
    
    /* Float condition variable */
    if (f > 1.0f) {
        f = f * 0.5f;  /* MODIFIES condition variable */
        result = g * 2.0f;
        global_accumulator += (int)result;
    } else {
        result = g / 2.0f;
        global_accumulator -= (int)result;
    }
    
    return result + f;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *v) {
    int local = *v;
    int result = 0;
    
    /* Condition uses volatile read */
    if (local > 50) {
        *v = 25;  /* MODIFIES volatile variable */
        result = local * 2;
        global_accumulator += result;
    } else {
        result = local + 10;
        global_accumulator -= result;
    }
    
    return result;
}

/* Test 6: Complex expression in condition */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int cond = a + b;
    int result = 0;
    
    /* Complex condition expression */
    if ((a > 0) && (b < 100) && (cond > 50)) {
        a = b;  /* MODIFIES part of condition expression */
        result = c * 3;
        global_accumulator += result;
    } else {
        result = c + 10;
        global_accumulator -= result;
    }
    
    return result + a + b;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O2")))
int test_loop_with_branch(int *arr, int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* This if-structure is a candidate for if-conversion */
        if (val > global_cond) {
            global_cond = val;  /* MODIFIES global used in condition */
            sum += val * 2;
        } else {
            sum += val;
        }
        
        /* Add unpredictability */
        if (rand() % 100 < 30) {
            arr[i] = val / 2;
        }
    }
    
    return sum;
}

/* Test 8: Multiple related if-statements */
__attribute__((noinline, optimize("O3")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First if - safe */
    if (x > 0) {
        result += y;
    } else {
        result -= y;
    }
    
    /* Second if - unsafe (modifies x) */
    if (x < 100) {
        x = x * 2;  /* MODIFIES condition variable for next if */
        result += z;
    } else {
        result -= z;
    }
    
    /* Third if - depends on modified x */
    if (x > 50) {
        result *= 2;
        global_accumulator += result;
    }
    
    return result;
}

/* Test 9: Using __builtin_expect to influence heuristics */
__attribute__((noinline, optimize("O2")))
int test_builtin_expect(int a, int b) {
    int result = 0;
    
    /* Hint that condition is likely true */
    if (__builtin_expect(a > 0, 1)) {
        a = b;  /* MODIFIES condition variable */
        result = a * 3;
        global_accumulator += result;
    } else {
        result = b + 5;
        global_accumulator -= result;
    }
    
    return result;
}

/* Test 10: Nested if with modification */
__attribute__((noinline, optimize("O3")))
int test_nested_branch(int p, int q) {
    int result = 0;
    
    if (p > 10) {
        if (q < p) {
            p = q;  /* MODIFIES outer condition variable */
            result = p * q;
        } else {
            result = p + q;
        }
        global_accumulator += result;
    } else {
        result = q - p;
        global_accumulator -= result;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, seed;
    int test_results[10] = {0};
    
    /* Use argv for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test variables */
    int x = rand() % 200 - 100;
    int y = rand() % 100;
    float f = (float)(rand() % 1000) / 10.0f;
    float g = (float)(rand() % 1000) / 10.0f;
    volatile int volatile_var = rand() % 100;
    int arr[20];
    
    for (i = 0; i < 20; i++) {
        arr[i] = rand() % 200;
    }
    
    /* Execute all test functions */
    test_results[0] = test_unsafe_modification(x, y);
    test_results[1] = test_safe_pattern(x, y);
    
    int ptr_val = rand() % 100;
    test_results[2] = test_pointer_condition(&ptr_val, 50);
    
    test_results[3] = (int)test_float_condition(f, g);
    test_results[4] = test_volatile_condition(&volatile_var);
    test_results[5] = test_complex_condition(x, y, rand() % 100);
    test_results[6] = test_loop_with_branch(arr, 20);
    test_results[7] = test_multiple_branches(x, y, rand() % 100);
    test_results[8] = test_builtin_expect(x, y);
    test_results[9] = test_nested_branch(x, y);
    
    /* Print results to ensure all code paths are used */
    printf("Seed: %d\n", seed);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Test results: ");
    for (i = 0; i < 10; i++) {
        printf("%d ", test_results[i]);
    }
    printf("\n");
    
    /* Use results to prevent optimization */
    int final_result = 0;
    for (i = 0; i < 10; i++) {
        final_result ^= test_results[i];
    }
    
    return final_result % 256;
}
