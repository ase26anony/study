#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for condition testing */
volatile int global_cond = 0;
int global_result = 0;
float global_float = 0.0f;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Force runtime evaluation */
    if (x > 0) {  /* Condition uses x */
        x = y * 2;  /* MODIFIES condition variable x in then block */
        result = 100;
    } else {
        result = 200;
    }
    
    /* Prevent dead code elimination */
    global_result += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int x, int y) {
    int result = 0;
    
    if (x < y) {  /* Condition uses x and y */
        result = x * 2;  /* Does NOT modify x or y */
    } else {
        result = y * 3;
    }
    
    global_result += result;
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    if (*ptr > threshold) {  /* Condition dereferences ptr */
        *ptr = threshold - 1;  /* MODIFIES the dereferenced value */
        result = 300;
    } else {
        result = 400;
    }
    
    global_result += result;
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((optimize("O3"), noinline))
float test_float_condition(float a, float b) {
    float result = 0.0f;
    
    if (a > b) {  /* Float condition */
        a = b * 2.0f;  /* MODIFIES condition variable a */
        result = a + 1.0f;
    } else {
        result = b - 1.0f;
    }
    
    global_float += result;
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((optimize("O2"), noinline))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    if (*v > 10) {  /* Volatile access in condition */
        *v = 5;  /* MODIFIES volatile variable */
        result = 500;
    } else {
        result = 600;
    }
    
    global_result += result;
    return result;
}

/* Test 6: Complex expression in condition with partial modification */
__attribute__((optimize("O3"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    if ((a + b) > c) {  /* Complex condition expression */
        a = c - b;  /* MODIFIES part of condition expression (a) */
        result = 700;
    } else {
        result = 800;
    }
    
    global_result += result;
    return result;
}

/* Test 7: Loop with if-conversion candidate and unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_loop_with_unsafe_mod(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int x = i % 10;
        int y = i % 5;
        
        /* This if should be considered for if-conversion */
        if (x > y) {  /* Condition uses x */
            x = y * 2;  /* MODIFIES condition variable x */
            sum += x;
        } else {
            sum += y;
        }
        
        /* Add some noise to prevent other optimizations */
        sum += (i & 1);
    }
    
    global_result += sum;
    return sum;
}

/* Test 8: Multiple if-statements in sequence */
__attribute__((optimize("O3"), noinline))
int test_multiple_ifs(int a, int b, int c) {
    int result = 0;
    
    /* First if - safe */
    if (a > 0) {
        result += 10;
    } else {
        result += 20;
    }
    
    /* Second if - unsafe (modifies b in then) */
    if (b < c) {
        b = a + c;  /* MODIFIES condition variable b */
        result += 30;
    } else {
        result += 40;
    }
    
    /* Third if - safe */
    if (c != 0) {
        result += 50;
    } else {
        result += 60;
    }
    
    global_result += result;
    return result;
}

/* Test 9: Using __builtin_expect to influence branch prediction */
__attribute__((optimize("O2"), noinline))
int test_builtin_expect(int x, int y) {
    int result = 0;
    
    if (__builtin_expect(x > y, 1)) {  /* Hint that x>y is likely */
        x = y - 1;  /* MODIFIES condition variable x */
        result = 900;
    } else {
        result = 1000;
    }
    
    global_result += result;
    return result;
}

/* Test 10: Global variable condition */
__attribute__((optimize("O3"), noinline))
int test_global_condition(int val) {
    int result = 0;
    
    if (global_cond > val) {
        global_cond = val;  /* MODIFIES global condition variable */
        result = 1100;
    } else {
        result = 1200;
    }
    
    global_result += result;
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv or time for randomness to prevent compile-time evaluation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize test variables with non-constant values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    int ptr_val = rand() % 100;
    int *ptr = &ptr_val;
    volatile int volatile_val = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    
    /* Call all test functions */
    int r1 = test_unsafe_modification(x, y);
    int r2 = test_safe_pattern(x, y);
    int r3 = test_pointer_condition(ptr, 50);
    float r4 = test_float_condition(f1, f2);
    int r5 = test_volatile_condition(&volatile_val);
    int r6 = test_complex_condition(x, y, z);
    int r7 = test_loop_with_unsafe_mod(100);
    int r8 = test_multiple_ifs(x, y, z);
    int r9 = test_builtin_expect(x, y);
    int r10 = test_global_condition(50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %.2f %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Global accumulators: result=%d float=%.2f\n", 
           global_result, global_float);
    
    return 0;
}
