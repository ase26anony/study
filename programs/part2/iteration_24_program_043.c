#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Force runtime evaluation */
    if (global_counter++ % 2) {
        x = rand() % 100;
    }
    
    /* Critical if-then-else: x is modified in then block */
    if (x > 50) {
        x = x + 10;  /* MODIFIES condition variable x */
        result = y * 2;
        global_accumulator += result;
    } else {
        result = y / 2;
        global_accumulator -= result;
    }
    
    /* Use x to prevent dead code elimination */
    return result + (x % 2);
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((noinline, optimize("O3")))
float test_safe_pattern(float a, float b) {
    float result = 0.0f;
    volatile float cond_var = a;  /* volatile to prevent constant folding */
    
    /* Multiple if-statements to create larger basic block */
    if (cond_var > 0.0f) {
        result = b * 3.14f;
        global_accumulator += (int)result;
    } else {
        result = b / 3.14f;
        global_accumulator -= (int)result;
    }
    
    /* Another if-statement using same variable but not modifying it */
    if (cond_var < 100.0f) {
        result += 1.0f;
    } else {
        result -= 1.0f;
    }
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int local_copy = *ptr;
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local_copy > threshold, 1)) {
        *ptr = local_copy + 5;  /* Modifies through pointer - affects condition */
        result = threshold * 3;
        global_accumulator ^= result;
    } else {
        result = threshold + 10;
        global_accumulator |= result;
    }
    
    return result;
}

/* Test 4: Mixed types with increment in then block */
__attribute__((noinline, optimize("O3")))
int test_increment_condition(int base, int mod) {
    int counter = base;
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        if (counter % mod == 0) {
            counter++;  /* MODIFIES condition variable counter */
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Additional computation to encourage if-conversion */
        result = (result > 100) ? result - 50 : result + 50;
    }
    
    return result + counter;
}

/* Test 5: Global variable condition with safe usage */
__attribute__((noinline, optimize("O2")))
int test_global_condition(int value) {
    int result = 0;
    int local_cond = global_cond;
    
    if (local_cond > value) {
        result = value * 10;
        global_accumulator += result;
        /* global_cond not modified here - safe */
    } else {
        result = value + 10;
        global_accumulator -= result;
    }
    
    /* Force another evaluation */
    if (local_cond < 0) {
        result = -result;
    }
    
    return result;
}

/* Test 6: Complex expression with array access */
__attribute__((noinline, optimize("O3")))
int test_array_condition(int *arr, int size) {
    int sum = 0;
    int idx = size / 2;
    
    /* Multiple if-statements in sequence */
    if (arr[idx] > 0) {
        idx = (idx + 1) % size;  /* Modifies idx used in next condition */
        sum += arr[idx];
    } else {
        sum -= arr[idx];
    }
    
    if (idx < size - 1) {
        arr[idx] = sum;  /* Modifies array element */
        sum *= 2;
    }
    
    return sum;
}

/* Test 7: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *vptr) {
    int result = 0;
    int local = *vptr;
    
    if (local > 1000) {
        *vptr = local - 500;  /* Modifies volatile location */
        result = 1;
    } else {
        result = 0;
    }
    
    /* Use result in computation */
    return result * 100 + local;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Testing if-conversion patterns (seed: %d)\n", seed);
    
    /* Initialize test data */
    int test_data[10];
    for (int i = 0; i < 10; i++) {
        test_data[i] = rand() % 200;
    }
    
    float float_data[5];
    for (int i = 0; i < 5; i++) {
        float_data[i] = (rand() % 1000) / 10.0f;
    }
    
    /* Execute test functions */
    int total = 0;
    
    total += test_unsafe_modification(rand() % 100, rand() % 100);
    total += (int)test_safe_pattern(float_data[0], float_data[1]);
    
    int ptr_val = rand() % 200;
    total += test_pointer_condition(&ptr_val, 100);
    
    total += test_increment_condition(rand() % 50, 7);
    
    global_cond = rand() % 200 - 100;
    total += test_global_condition(rand() % 100);
    
    total += test_array_condition(test_data, 10);
    
    volatile int volatile_var = rand() % 2000;
    total += test_volatile_condition(&volatile_var);
    
    /* Additional mixed tests in a loop */
    for (int i = 0; i < 5; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        
        /* Alternate between safe and unsafe patterns */
        if (i % 2 == 0) {
            total += test_unsafe_modification(a, b);
        } else {
            total += test_safe_pattern((float)a, (float)b);
        }
    }
    
    printf("Final result: %d (accumulator: %d)\n", total, global_accumulator);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
