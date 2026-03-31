#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;
volatile int global_flag = 0;

/* Test 1: Unsafe modification of condition variable in then-block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in then-block */
        if (x > y) {
            x = x + 1;  /* MODIFIES condition variable */
            result += i * 10;
            global_acc += 1;
        } else {
            result += i * 5;
            global_acc -= 1;
        }
        
        /* Mix with other operations to encourage if-conversion */
        y = y ^ (i + 1);
    }
    
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(float a, float b) {
    float temp = a;
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(a > b, 1)) {
        /* Does NOT modify a or b */
        result = (int)(a * 100.0f);
        global_acc += result;
    } else {
        result = (int)(b * 50.0f);
        global_acc -= result;
        temp = b;  /* Modify local copy, not condition variable */
    }
    
    return result;
}

/* Test 3: Pointer dereference condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr1, int *ptr2) {
    int local = 0;
    
    /* Multiple if-statements in sequence */
    if (*ptr1 > *ptr2) {
        (*ptr1)++;  /* MODIFIES dereferenced condition variable */
        local += 10;
        global_flag = 1;
    } else {
        local += 20;
        global_flag = 2;
    }
    
    if (*ptr2 < 100) {
        /* Another condition */
        local *= 2;
    }
    
    return local;
}

/* Test 4: Volatile condition variable with modification */
__attribute__((optimize("O3"), noinline))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    /* Use runtime value to prevent constant folding */
    if (*v > 0) {
        (*v) = (*v) - 1;  /* MODIFIES volatile condition variable */
        result = 100;
        global_acc += 100;
    } else {
        result = 200;
        global_acc += 200;
    }
    
    return result;
}

/* Test 5: Complex expression in condition with partial modification */
__attribute__((optimize("O2"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    
    /* Complex condition */
    if ((a + b) > (c * 2)) {
        a = b + c;  /* Modifies 'a' which is part of condition */
        result = a * 2;
        global_acc ^= result;
    } else {
        result = b * 3;
        global_acc ^= result;
    }
    
    /* Second if to create larger basic block */
    if (result > 50) {
        result /= 2;
    }
    
    return result;
}

/* Test 6: Safe nested if with no modification of condition */
__attribute__((optimize("O3"), noinline))
int test_safe_nested(int x, int y, int z) {
    int result = x;
    
    if (x > 0) {
        if (y > 0) {
            /* No modification of x or y */
            result = z * 2;
            global_acc += z;
        } else {
            result = z / 2;
            global_acc -= z;
        }
        /* Still not modifying x or y */
        result += 10;
    } else {
        result = -1;
    }
    
    return result;
}

/* Test 7: Mixed modification pattern */
__attribute__((optimize("O2"), noinline))
int test_mixed_modification(int *arr, int idx) {
    int val = arr[idx];
    int sum = 0;
    
    /* Loop with if-conversion candidate */
    for (int i = 0; i < 4; i++) {
        if (val > arr[i]) {
            val = arr[i];  /* MODIFIES condition variable */
            sum += i * 10;
        } else {
            sum += i * 5;
        }
        
        /* Prevent loop unrolling from eliminating if */
        arr[i] += (i % 2);
    }
    
    return sum;
}

/* Main driver with runtime-dependent paths */
int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    int total_result = 0;
    
    /* Initialize test variables with runtime values */
    int x = rand() % 100;
    int y = rand() % 100;
    float a = (float)(rand() % 100) / 10.0f;
    float b = (float)(rand() % 100) / 10.0f;
    
    int ptr_val1 = rand() % 50;
    int ptr_val2 = rand() % 50;
    int *ptr1 = &ptr_val1;
    int *ptr2 = &ptr_val2;
    
    volatile int volatile_val = rand() % 20;
    volatile int *vptr = &volatile_val;
    
    int arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i] = rand() % 30;
    }
    
    /* Execute all test functions */
    total_result += test_unsafe_modification(x, y);
    total_result += test_safe_pattern(a, b);
    total_result += test_pointer_condition(ptr1, ptr2);
    total_result += test_volatile_condition(vptr);
    total_result += test_complex_condition(x, y, ptr_val1);
    total_result += test_safe_nested(x, y, ptr_val2);
    total_result += test_mixed_modification(arr, rand() % 4);
    
    /* Use results to prevent dead code elimination */
    printf("Seed: %d\n", seed);
    printf("Total result: %d\n", total_result);
    printf("Global accumulator: %d\n", global_acc);
    printf("Global flag: %d\n", global_flag);
    
    return total_result != 0 ? 0 : 1;
}
