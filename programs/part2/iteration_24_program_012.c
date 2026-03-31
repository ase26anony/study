#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for condition testing */
volatile int global_cond = 0;
int global_result = 0;
float global_float = 0.0f;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Force runtime evaluation */
    if (x > 0) {  /* Condition uses x */
        x = y + 10;  /* MODIFIES condition variable x in then-block */
        result = 100;
    } else {
        result = 200;
    }
    
    /* Prevent dead code elimination */
    global_result += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
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
        *ptr = threshold - 1;  /* MODIFIES dereferenced value */
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
    
    if (*v == 0) {  /* Volatile access */
        *v = 1;  /* MODIFIES volatile */
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
    
    /* Complex condition expression */
    if ((a + b) > (c * 2)) {
        b = c + 5;  /* MODIFIES b which is part of condition */
        result = 700;
    } else {
        result = 800;
    }
    
    global_result += result;
    return result;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((optimize("O2"), noinline))
int test_loop_with_branch(int n, int *data) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* This if may be if-converted */
        if (data[i] > 0) {
            data[i] = -data[i];  /* MODIFIES condition source */
            sum += 1;
        } else {
            sum += 2;
        }
    }
    
    global_result += sum;
    return sum;
}

/* Test 8: Multiple related if-statements */
__attribute__((optimize("O3"), noinline))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First branch - safe */
    if (x > 0) {
        result += 10;
    } else {
        result += 20;
    }
    
    /* Second branch - unsafe (modifies y used in next condition) */
    if (y < 10) {
        y = 15;  /* MODIFIES y */
        result += 30;
    } else {
        result += 40;
    }
    
    /* Third branch - uses modified y */
    if (y > 5) {  /* y was potentially modified */
        result += 50;
    } else {
        result += 60;
    }
    
    global_result += result;
    return result;
}

/* Test 9: __builtin_expect with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_builtin_expect(int x) {
    int result = 0;
    
    if (__builtin_expect(x > 100, 1)) {
        x = 50;  /* MODIFIES condition variable */
        result = 900;
    } else {
        result = 1000;
    }
    
    global_result += result;
    return result;
}

/* Test 10: Nested if with outer condition modification */
__attribute__((optimize("O3"), noinline))
int test_nested_branch(int a, int b) {
    int result = 0;
    
    if (a > b) {
        if (a < 100) {
            a = b;  /* MODIFIES outer condition variable */
            result = 1100;
        } else {
            result = 1200;
        }
    } else {
        result = 1300;
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
    
    /* Initialize test variables */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    int ptr_val = rand() % 100;
    volatile int volatile_val = rand() % 10;
    
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = rand() % 200 - 100;
    }
    
    /* Execute all test functions */
    int r1 = test_unsafe_modification(x, y);
    int r2 = test_safe_pattern(x, y);
    int r3 = test_pointer_condition(&ptr_val, 50);
    float r4 = test_float_condition(f1, f2);
    int r5 = test_volatile_condition(&volatile_val);
    int r6 = test_complex_condition(x, y, z);
    int r7 = test_loop_with_branch(10, data);
    int r8 = test_multiple_branches(x, y, z);
    int r9 = test_builtin_expect(x);
    int r10 = test_nested_branch(x, y);
    
    /* Print results to prevent optimization */
    printf("Seed: %d\n", seed);
    printf("Results: %d %d %d %.2f %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Global accumulators: result=%d float=%.2f\n", 
           global_result, global_float);
    
    return 0;
}
