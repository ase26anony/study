#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_accumulator = 0;
int global_cond = 0;

/* Function to create side effects and prevent dead code elimination */
static void use_value(int val) {
    global_accumulator += val;
}

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in the then block */
        if (x > 0) {
            x = x + y;  /* MODIFIES condition variable x */
            result += 10;
        } else {
            result += 5;
        }
        
        /* Mix with other operations to encourage if-conversion */
        int temp = (x > y) ? x : y;
        result += temp;
    }
    
    use_value(result);
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int result = 0;
    int cond = a;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(cond > 0, 1)) {
        /* Does NOT modify cond */
        result = b * 2;
    } else {
        result = b / 2;
    }
    
    /* Multiple related if-statements */
    if (cond < 100) {
        result += 1;
    }
    
    if (b != 0) {
        result += 2;
    }
    
    use_value(result);
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    /* Use volatile to prevent reordering */
    volatile int *vptr = ptr;
    
    /* Condition based on pointer dereference */
    if (*vptr > threshold) {
        *vptr = threshold;  /* MODIFIES the dereferenced value */
        result = 100;
    } else {
        result = 50;
    }
    
    /* Additional operation to create candidate for conditional move */
    int diff = (*vptr > 0) ? *vptr : -(*vptr);
    result += diff;
    
    use_value(result);
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float inc) {
    float result = 0.0f;
    
    /* Prevent constant folding with runtime value */
    if (f > 0.0f) {
        f += inc;  /* MODIFIES condition variable f */
        result = f * 2.0f;
    } else {
        result = f / 2.0f;
    }
    
    /* Use result to prevent elimination */
    global_counter += (int)result;
    return result;
}

/* Test 5: Complex expression in condition with partial modification */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    int x = a + b;
    
    /* Complex condition */
    if ((x > c) && (a < 100)) {
        x = b * 2;  /* MODIFIES x which is part of condition */
        result = x + a;
    } else {
        result = c - a;
    }
    
    /* Nested if to create larger basic block */
    if (result > 0) {
        result *= 2;
    }
    
    use_value(result);
    return result;
}

/* Test 6: Safe pattern with global variable */
__attribute__((noinline, optimize("O3")))
int test_global_condition(int val) {
    int result = 0;
    
    /* Global variable in condition */
    if (global_cond > val) {
        /* Safe: doesn't modify global_cond */
        result = global_cond * 2;
    } else {
        result = val * 3;
    }
    
    /* Modify global after condition check */
    global_cond += result;
    
    use_value(result);
    return result;
}

/* Test 7: Array element as condition with modification */
__attribute__((noinline, optimize("O2")))
int test_array_condition(int arr[], int idx) {
    int result = 0;
    
    if (arr[idx] > 0) {
        arr[idx] = 0;  /* MODIFIES array element used in condition */
        result = 100;
    } else {
        result = 200;
    }
    
    /* Additional computation */
    result += (arr[idx] < 10) ? 5 : 10;
    
    use_value(result);
    return result;
}

/* Test 8: Mixed types and operations */
__attribute__((noinline, optimize("O3")))
int test_mixed_operations(int base, int mod) {
    int result = base;
    volatile int v = base % mod;
    
    /* Use volatile to prevent optimization */
    if (v > (mod / 2)) {
        v = v * 2;  /* MODIFIES volatile condition variable */
        result += v;
    } else {
        result -= v;
    }
    
    /* Conditional expression that might be converted to conditional move */
    int adjusted = (result > 0) ? result : -result;
    
    use_value(adjusted);
    return adjusted;
}

/* Main driver with runtime-dependent paths */
int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    int total = 0;
    
    /* Initialize test variables with runtime values */
    int x = rand() % 100;
    int y = rand() % 100;
    int a = rand() % 100;
    int b = rand() % 100;
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 200 - 100;
    }
    
    /* Call test functions - order matters for coverage */
    total += test_unsafe_modification(x, y);      /* Should trigger safety check */
    total += test_safe_pattern(a, b);             /* Should allow if-conversion */
    total += test_pointer_condition(&x, 50);      /* Should trigger safety check */
    total += (int)test_float_condition((float)x / 10.0f, 1.5f); /* Should trigger safety check */
    total += test_complex_condition(x, y, a);     /* Should trigger safety check */
    total += test_global_condition(b);            /* Should allow if-conversion */
    total += test_array_condition(arr, 3);        /* Should trigger safety check */
    total += test_mixed_operations(x + y, 7);     /* Should trigger safety check */
    
    /* Use results to prevent dead code elimination */
    printf("Total: %d\n", total);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Global counter: %d\n", global_counter);
    
    return total > 0 ? 0 : 1;
}
