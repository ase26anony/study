#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for condition testing */
volatile int global_cond = 0;
int global_result = 0;

/* Function prototypes */
int __attribute__((noinline)) test_unsafe_int_modification(int x, int y);
int __attribute__((noinline)) test_safe_pattern(int x, int y);
int __attribute__((noinline)) test_volatile_condition(volatile int* ptr);
int __attribute__((noinline)) test_float_condition(float f, float threshold);
int __attribute__((noinline)) test_pointer_deref(int* ptr, int limit);
int __attribute__((noinline)) test_mixed_conditions(int a, int b, int c);
int __attribute__((noinline)) test_nested_if(int x, int y, int z);
int __attribute__((optimize("O3"))) test_aggressive_opt(int x, int y);
int __attribute__((optimize("O2"))) test_O2_optimization(int x, int y);

/* Test 1: Unsafe - modifies condition variable in then block */
int __attribute__((noinline)) test_unsafe_int_modification(int x, int y) {
    int result = 0;
    
    /* This should trigger the safety check - x is modified in then block */
    if (x > 0) {  /* test_expr = x > 0 */
        x = y * 2;  /* MODIFIES condition variable x */
        result = 100;
    } else {
        result = -100;
    }
    
    /* Use x to prevent dead code elimination */
    return result + (x % 2);
}

/* Test 2: Safe - does not modify condition variable */
int __attribute__((noinline)) test_safe_pattern(int x, int y) {
    int result = 0;
    
    /* This should pass the safety check - x is not modified */
    if (x > y) {  /* test_expr = x > y */
        result = x + y;  /* Doesn't modify x or y */
    } else {
        result = x - y;
    }
    
    /* Add loop to encourage if-conversion */
    for (int i = 0; i < 10; i++) {
        if (__builtin_expect((result + i) > 50, 0)) {
            result -= i;
        } else {
            result += i;
        }
    }
    
    return result;
}

/* Test 3: Volatile condition variable */
int __attribute__((noinline)) test_volatile_condition(volatile int* ptr) {
    int local = *ptr;
    int result = 0;
    
    /* Volatile read in condition */
    if (*ptr > 10) {  /* test_expr = *ptr > 10 */
        *ptr = local + 5;  /* Modifies through pointer - should fail safety check */
        result = 1;
    } else {
        result = 0;
    }
    
    return result;
}

/* Test 4: Float condition with modification */
int __attribute__((noinline)) test_float_condition(float f, float threshold) {
    float original = f;
    int result = 0;
    
    /* Float comparison */
    if (f > threshold) {  /* test_expr = f > threshold */
        f = f * 1.5f;  /* Modifies condition variable f */
        result = (int)(f * 100);
    } else {
        result = (int)(f * 50);
    }
    
    /* Use f to prevent elimination */
    global_result += (int)f;
    return result;
}

/* Test 5: Pointer dereference in condition */
int __attribute__((noinline)) test_pointer_deref(int* ptr, int limit) {
    int result = 0;
    
    /* Complex condition with pointer dereference */
    if (*ptr < limit && ptr[1] > 0) {  /* test_expr involves *ptr */
        *ptr = limit;  /* Modifies what condition depends on */
        result = *ptr * 2;
    } else {
        result = limit / 2;
    }
    
    /* Multiple related if-statements */
    if (result > 100) {
        result = 100;
    }
    
    return result;
}

/* Test 6: Mixed conditions with global variable */
int __attribute__((noinline)) test_mixed_conditions(int a, int b, int c) {
    int result = 0;
    
    /* Condition using global */
    if (global_cond > a) {  /* test_expr = global_cond > a */
        global_cond = b;  /* Modifies global used in condition */
        result = a + b + c;
    } else {
        result = a - b - c;
    }
    
    /* Another if to create basic block region */
    if (result < 0) {
        result = -result;
    }
    
    return result;
}

/* Test 7: Nested if with modification in inner block */
int __attribute__((noinline)) test_nested_if(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {  /* Outer condition */
        if (y > x) {  /* Inner condition - y is condition variable */
            y = z * 2;  /* Modifies inner condition variable y */
            result = 1;
        } else {
            result = 2;
        }
        x = result;  /* Modifies outer condition variable x */
    } else {
        result = 3;
    }
    
    return result + x + y;
}

/* Test 8: Aggressive optimization with O3 */
int __attribute__((optimize("O3"))) test_aggressive_opt(int x, int y) {
    int result = 0;
    
    /* Pattern that looks good for conditional move */
    if (x != y) {  /* test_expr = x != y */
        x = x ^ y;  /* Modifies x which is in condition */
        result = x * y;
    } else {
        result = x + y;
    }
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 8; i++) {
        result += (result % (i + 2));
    }
    
    return result;
}

/* Test 9: O2 optimization with increment modification */
int __attribute__((optimize("O2"))) test_O2_optimization(int x, int y) {
    int result = 0;
    
    /* Increment modifies condition variable */
    if (x++ > y) {  /* test_expr = x > y, but x is incremented after test */
        /* Actually x was already incremented, but in RTL this might be
           seen as modification of the register holding x */
        result = x * 2;
    } else {
        result = y * 2;
    }
    
    /* Explicit modification in then block */
    if (result > 100) {
        x += 10;  /* x is in previous condition */
        result -= 50;
    }
    
    return result + x;
}

/* Main driver with runtime variability */
int main(int argc, char** argv) {
    int seed = 0;
    
    /* Use argv for runtime variability to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = rand();
    }
    
    srand(seed);
    
    /* Initialize test variables with runtime values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 1000) / 10.0f;
    int array[2] = {rand() % 50, rand() % 50};
    
    int total = 0;
    
    /* Call all test functions */
    total += test_unsafe_int_modification(x, y);
    total += test_safe_pattern(x, y);
    
    global_cond = rand() % 20;
    total += test_volatile_condition(&global_cond);
    
    total += test_float_condition(f, 50.0f);
    total += test_pointer_deref(array, 40);
    total += test_mixed_conditions(x, y, z);
    total += test_nested_if(x, y, z);
    total += test_aggressive_opt(x, y);
    total += test_O2_optimization(x, y);
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %d (seed: %d)\n", total, seed);
    
    /* Additional loop with if-conversion candidate */
    int loop_result = 0;
    for (int i = 0; i < 100; i++) {
        int temp = rand() % 100;
        if (temp > 50) {  /* test_expr = temp > 50 */
            temp = i * 2;  /* Modifies condition variable */
            loop_result += temp;
        } else {
            loop_result -= temp;
        }
    }
    
    printf("Loop result: %d\n", loop_result);
    
    return total > 0 ? 0 : 1;
}
