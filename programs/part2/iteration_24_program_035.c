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
    
    if (x > 0) {  /* Condition uses x */
        y = x * 3;  /* Modifies y, NOT x */
        result = 300;
    } else {
        result = 400;
    }
    
    global_result += result;
    return result;
}

/* Test 3: Pointer dereference condition with modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    if (*ptr > threshold) {  /* Condition uses *ptr */
        *ptr = threshold - 1;  /* MODIFIES dereferenced pointer in then block */
        result = 500;
    } else {
        result = 600;
    }
    
    global_result += result;
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((optimize("O3"), noinline))
int test_float_modification(float f, float inc) {
    int result = 0;
    
    if (f > 1.0f) {  /* Condition uses f */
        f += inc;  /* MODIFIES condition variable f in then block */
        global_float = f;
        result = 700;
    } else {
        result = 800;
    }
    
    global_result += result;
    return result;
}

/* Test 5: Volatile variable condition */
__attribute__((optimize("O2"), noinline))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    if (*v > 10) {  /* Condition uses volatile *v */
        *v = 5;  /* MODIFIES volatile variable in then block */
        result = 900;
    } else {
        result = 1000;
    }
    
    global_result += result;
    return result;
}

/* Test 6: Complex expression in condition with modification */
__attribute__((optimize("O3"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    if ((a + b) > c) {  /* Condition uses a and b */
        a = b + c;  /* MODIFIES a which is part of condition expression */
        result = 1100;
    } else {
        result = 1200;
    }
    
    global_result += result;
    return result;
}

/* Test 7: Loop with if-conversion candidate (safe) */
__attribute__((optimize("O2"), noinline))
int test_loop_safe(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int x = i * 2;
        int y = i + 5;
        
        if (x > y) {  /* Condition uses x and y */
            sum += x - y;  /* Doesn't modify x or y */
        } else {
            sum += y - x;
        }
    }
    
    global_result += sum;
    return sum;
}

/* Test 8: Loop with if-conversion candidate (unsafe) */
__attribute__((optimize("O3"), noinline))
int test_loop_unsafe(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int x = i * 2;
        int y = i + 5;
        
        if (x > y) {  /* Condition uses x */
            x = y * 3;  /* MODIFIES x in then block */
            sum += x;
        } else {
            sum += y;
        }
    }
    
    global_result += sum;
    return sum;
}

/* Test 9: Multiple if-statements in sequence */
__attribute__((optimize("O2"), noinline))
int test_multiple_ifs(int a, int b, int c) {
    int result = 0;
    
    /* First if - safe */
    if (a > 0) {
        result += 10;
    } else {
        result += 20;
    }
    
    /* Second if - unsafe */
    if (b > c) {  /* Condition uses b */
        b = a + c;  /* MODIFIES b in then block */
        result += 30;
    } else {
        result += 40;
    }
    
    /* Third if - safe with __builtin_expect */
    if (__builtin_expect(c > 0, 1)) {
        result += 50;
    } else {
        result += 60;
    }
    
    global_result += result;
    return result;
}

/* Test 10: Nested if with modification */
__attribute__((optimize("O3"), noinline))
int test_nested_modification(int x, int y, int z) {
    int result = 0;
    
    if (x > y) {  /* Outer condition uses x */
        if (y > z) {  /* Inner condition uses y */
            x = z * 2;  /* MODIFIES x (from outer condition) in inner then block */
            result = 70;
        } else {
            result = 80;
        }
    } else {
        result = 90;
    }
    
    global_result += result;
    return result;
}

int main(int argc, char *argv[]) {
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
    float f = (float)(rand() % 100) / 10.0f;
    int arr[3] = {rand() % 100, rand() % 100, rand() % 100};
    volatile int volatile_var = rand() % 100;
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    printf("Initial values: x=%d, y=%d, z=%d, f=%.2f\n", x, y, z, f);
    
    /* Call all test functions to exercise different patterns */
    int r1 = test_unsafe_modification(x, y);
    int r2 = test_safe_pattern(x, y);
    int r3 = test_pointer_condition(&arr[0], 50);
    int r4 = test_float_modification(f, 2.5f);
    int r5 = test_volatile_condition(&volatile_var);
    int r6 = test_complex_condition(x, y, z);
    int r7 = test_loop_safe(10);
    int r8 = test_loop_unsafe(10);
    int r9 = test_multiple_ifs(x, y, z);
    int r10 = test_nested_modification(x, y, z);
    
    /* Use results to prevent dead code elimination */
    int total = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    printf("Individual results: %d %d %d %d %d %d %d %d %d %d\n",
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Global accumulator: %d\n", global_result);
    printf("Total: %d\n", total);
    printf("Final float value: %.2f\n", global_float);
    
    return total > 0 ? 0 : 1;
}
