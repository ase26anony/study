#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimizations */
volatile int global_counter = 0;
int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in then-block */
        if (x > y) {
            x = x + 1;  /* MODIFIES condition variable! */
            result += 100;
        } else {
            result += 50;
        }
        
        /* Mix with other operations */
        y = y * 2 - 1;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int local = a;
    int sum = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(a > b, 1)) {
        /* Does NOT modify a or b */
        sum = a * 2 + 10;
    } else {
        sum = b * 3 - 5;
    }
    
    /* Additional if-statement to create larger basic block */
    if (local < 100) {
        sum += 20;
    } else {
        sum -= 10;
    }
    
    global_counter++;
    return sum;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int val = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold;  /* MODIFIES the dereferenced value! */
        val = 1000;
    } else {
        val = 500;
    }
    
    /* Nested if to encourage if-conversion */
    if (val > 750) {
        global_accumulator += 100;
    }
    
    return val;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f1, float f2) {
    float result = 0.0f;
    
    /* Use volatile to prevent reordering */
    volatile float cond = f1;
    
    if (cond > f2) {
        cond = f2;  /* MODIFIES condition variable */
        result = f1 * 2.0f;
    } else {
        result = f2 * 3.0f;
    }
    
    return result;
}

/* Test 5: Complex condition expression */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int x, int y, int z) {
    int result = 0;
    
    /* Complex condition */
    if ((x > y) && (x < z)) {
        x = y + z;  /* MODIFIES x which is part of condition */
        result = x * 2;
    } else {
        result = y + z;
    }
    
    /* Multiple related if-statements */
    if (result > 100) {
        result -= 50;
    }
    
    return result;
}

/* Test 6: Safe with volatile condition */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int *vptr) {
    int a = 0;
    
    /* Condition uses volatile read */
    if (*vptr > 0) {
        /* Safe: doesn't modify *vptr */
        a = *vptr * 2;
    } else {
        a = -*vptr;
    }
    
    return a;
}

/* Test 7: Array-based condition with modification */
__attribute__((noinline, optimize("O2")))
int test_array_condition(int arr[], int idx) {
    int total = 0;
    
    /* Condition based on array element */
    if (arr[idx] > 0) {
        arr[idx] = 0;  /* MODIFIES the array element used in condition */
        total = 100;
    } else {
        total = 200;
    }
    
    return total;
}

/* Test 8: Mixed types and operations */
__attribute__((optimize("O3")))
void test_mixed_operations(int seed) {
    int a = seed;
    int b = seed * 2;
    int c = 0;
    
    /* Sequence of if-statements */
    if (a % 2 == 0) {
        a++;  /* Modifies a, but next condition uses b */
        c += 10;
    }
    
    if (b > a) {
        /* Safe: doesn't modify b or a */
        c += 20;
    } else {
        b = a;  /* Modifies b */
        c += 30;
    }
    
    global_accumulator += c;
}

/* Main driver with runtime-dependent paths */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) % 100;
    }
    
    srand(seed);
    
    /* Initialize test variables */
    int x = rand() % 100;
    int y = rand() % 100;
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 200 - 100;
    }
    
    volatile int volatile_var = rand() % 50;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    
    int *ptr = &x;
    
    printf("Starting tests with seed: %d\n", seed);
    
    /* Execute all test patterns */
    int r1 = test_unsafe_modification(x, y);
    printf("Test 1 result: %d\n", r1);
    
    int r2 = test_safe_pattern(x, y);
    printf("Test 2 result: %d\n", r2);
    
    int r3 = test_pointer_condition(ptr, 50);
    printf("Test 3 result: %d\n", r3);
    
    float r4 = test_float_condition(f1, f2);
    printf("Test 4 result: %.2f\n", r4);
    
    int r5 = test_complex_condition(x, y, seed);
    printf("Test 5 result: %d\n", r5);
    
    int r6 = test_volatile_condition(&volatile_var);
    printf("Test 6 result: %d\n", r6);
    
    int r7 = test_array_condition(arr, 3);
    printf("Test 7 result: %d\n", r7);
    
    test_mixed_operations(seed);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
