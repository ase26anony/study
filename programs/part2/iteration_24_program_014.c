#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in then block */
        if (x > y) {
            x = x + 1;  /* MODIFIES condition variable */
            result += 100;
        } else {
            result += 50;
        }
        
        /* Additional code to make block non-trivial */
        y = y * 2 + 1;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int a, int b) {
    int local_result = 0;
    int cond_var = a;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(cond_var < b, 1)) {
        /* Does NOT modify cond_var */
        local_result = b * 2;
    } else {
        local_result = a * 3;
    }
    
    /* Modify cond_var outside the condition */
    cond_var += local_result;
    
    global_accumulator += local_result;
    return local_result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int threshold) {
    int val = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold;  /* MODIFIES the dereferenced value */
        val = 1000;
    } else {
        val = 500;
    }
    
    /* Additional if-statement to create larger basic block */
    if (val > 600) {
        global_cond = 1;
    }
    
    global_accumulator += val;
    return val;
}

/* Test 4: Float condition with modification */
__attribute__((optimize("O3"), noinline))
float test_float_condition(float f1, float f2) {
    float result_f = 0.0f;
    
    /* Float comparison */
    if (f1 > f2) {
        f1 = f2 * 2.0f;  /* MODIFIES condition variable */
        result_f = f1 + 10.0f;
    } else {
        result_f = f2 - 5.0f;
    }
    
    global_accumulator += (int)result_f;
    return result_f;
}

/* Test 5: Volatile condition variable */
__attribute__((optimize("O2"), noinline))
int test_volatile_condition(volatile int *vptr) {
    int res = 0;
    
    /* Multiple related if-statements */
    if (*vptr > 100) {
        *vptr = 50;  /* MODIFIES volatile condition */
        res = 1;
    }
    
    if (*vptr < 20) {
        res += 2;
    }
    
    global_accumulator += res;
    return res;
}

/* Test 6: Nested conditions with mixed safety */
__attribute__((optimize("O3"), noinline))
int test_nested_conditions(int x, int y, int z) {
    int total = 0;
    
    /* Outer condition - safe */
    if (x > 0) {
        /* Inner condition - UNSAFE (modifies y) */
        if (y < z) {
            y = x + z;  /* MODIFIES condition variable of inner if */
            total += 100;
        } else {
            total += 200;
        }
        
        /* Another safe condition */
        if (z > 10) {
            total += 50;
        }
    } else {
        total = 300;
    }
    
    global_accumulator += total;
    return total;
}

/* Test 7: Complex expression in condition */
__attribute__((optimize("O2"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition expression */
    if ((a * b) > (c + 10)) {
        a = b + c;  /* MODIFIES 'a' which is part of condition */
        result = a * 2;
    } else {
        result = c * 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 8: Safe modification in else block (should allow if-conversion) */
__attribute__((optimize("O3"), noinline))
int test_safe_else_modification(int p, int q) {
    int output = 0;
    int cond = p;
    
    if (cond > q) {
        output = 100;
    } else {
        cond = q + 5;  /* Modifies in ELSE block, not THEN */
        output = 200;
    }
    
    global_accumulator += output;
    return output;
}

/* Test 9: Array element as condition */
__attribute__((optimize("O2"), noinline))
int test_array_condition(int arr[], int idx) {
    int sum = 0;
    
    if (arr[idx] > 0) {
        arr[idx] = -1;  /* MODIFIES array element used in condition */
        sum = 100;
    } else {
        sum = 200;
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 10: Mixed scenario with loop */
__attribute__((optimize("O3"), noinline))
int test_mixed_with_loop(int base, int iterations) {
    int counter = base;
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* This condition's variable gets modified in the loop body */
        if (counter > i) {
            counter--;  /* MODIFIES condition variable */
            accum += 10;
        } else {
            accum += 5;
        }
        
        /* Additional computation to prevent simplification */
        accum += (i % 3);
    }
    
    global_accumulator += accum;
    return accum;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line or time for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize test variables with random-ish values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 200 - 100;
    }
    
    volatile int volatile_var = rand() % 150;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    
    int *ptr = &x;
    volatile int *vptr = &volatile_var;
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    
    /* Execute all test functions */
    int result1 = test_unsafe_modification(x, y);
    int result2 = test_safe_pattern(y, z);
    int result3 = test_pointer_condition(ptr, 50);
    float result4 = test_float_condition(f1, f2);
    int result5 = test_volatile_condition(vptr);
    int result6 = test_nested_conditions(x, y, z);
    int result7 = test_complex_condition(x, y, z);
    int result8 = test_safe_else_modification(y, z);
    int result9 = test_array_condition(arr, 3);
    int result10 = test_mixed_with_loop(x, 5);
    
    printf("Results: %d, %d, %d, %.2f, %d, %d, %d, %d, %d, %d\n",
           result1, result2, result3, result4, result5,
           result6, result7, result8, result9, result10);
    
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Global condition flag: %d\n", global_cond);
    
    return global_accumulator > 0 ? 0 : 1;
}
