#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in then-block */
        if (x > 0) {
            x = x + y;  /* MODIFIES condition variable! */
            result += 10;
            global_acc++;
        } else {
            result -= 5;
            global_acc--;
        }
        
        /* Mix with other operations to encourage if-conversion */
        y = y * 2 + 1;
    }
    
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int local_cond = a;
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local_cond > 100, 1)) {
        /* Does NOT modify local_cond */
        result = b * 2;
        global_acc += result;
    } else {
        result = b / 2;
        global_acc -= result;
        local_cond = b;  /* Modify here, but not in then-block */
    }
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int temp = 0;
    
    /* Multiple if-statements in sequence */
    if (*ptr < threshold) {
        *ptr += 10;  /* Modifies dereferenced condition expression */
        temp = 1;
        global_acc += 100;
    } else {
        temp = -1;
        global_acc -= 50;
    }
    
    if (threshold > 0) {
        temp *= 2;
    }
    
    return temp;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float inc) {
    float result = 0.0f;
    
    /* Use runtime value to prevent constant folding */
    if (f > 0.5f) {
        f += inc;  /* Modifies float condition variable */
        result = f * 2.0f;
        global_acc += (int)result;
    } else {
        result = f / 2.0f;
        global_acc -= (int)result;
    }
    
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_cond(volatile int *v) {
    int res = 0;
    
    /* Loop with volatile read */
    for (int i = 0; i < 2; i++) {
        if (*v > 0) {
            (*v)++;  /* Modifies volatile condition variable */
            res += 20;
            global_acc += 5;
        } else {
            res -= 10;
            global_acc -= 3;
        }
    }
    
    return res;
}

/* Test 6: Complex expression in condition with partial modification */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int cond_base = a + b;
    int result = 0;
    
    /* Complex condition */
    if ((cond_base * c) > 1000) {
        /* Only modifies part of the condition expression (c) */
        c = c * 2;  /* Modifies variable used in condition */
        result = cond_base + c;
        global_acc += result;
    } else {
        result = cond_base - c;
        global_acc -= result;
        cond_base = b;  /* Safe modification here */
    }
    
    return result;
}

/* Test 7: Nested if with modification in inner block */
__attribute__((noinline, optimize("O2")))
int test_nested_modification(int x, int y, int z) {
    int total = 0;
    
    if (x > y) {
        if (z > 0) {
            x = z;  /* Modifies outer condition variable */
            total = x + y;
            global_acc += total;
        } else {
            total = x - y;
            global_acc -= total;
        }
    } else {
        total = y - x;
    }
    
    return total;
}

/* Test 8: Safe function that should allow if-conversion */
__attribute__((noinline, optimize("O3")))
int test_safe_conversion(int a, int b) {
    int result;
    
    /* Simple pattern good for conditional move */
    if (a > b) {
        result = a - b;
    } else {
        result = b - a;
    }
    
    global_acc += result;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    int cond_var2 = rand() % 200;
    float cond_var3 = (float)(rand() % 1000) / 1000.0f;
    volatile int cond_var4 = rand() % 50;
    int ptr_val = rand() % 300;
    int *ptr = &ptr_val;
    
    printf("Starting tests with seed: %d\n", seed);
    printf("Initial global_acc: %d\n", global_acc);
    
    /* Call test functions - mix safe and unsafe patterns */
    int r1 = test_unsafe_modification(cond_var1, cond_var2);
    printf("Test1 result: %d, global_acc: %d\n", r1, global_acc);
    
    int r2 = test_safe_pattern(cond_var1, cond_var2);
    printf("Test2 result: %d, global_acc: %d\n", r2, global_acc);
    
    int r3 = test_pointer_condition(ptr, 150);
    printf("Test3 result: %d, global_acc: %d\n", r3, global_acc);
    
    float r4 = test_float_condition(cond_var3, 0.1f);
    printf("Test4 result: %.2f, global_acc: %d\n", r4, global_acc);
    
    int r5 = test_volatile_cond(&cond_var4);
    printf("Test5 result: %d, global_acc: %d\n", r5, global_acc);
    
    int r6 = test_complex_condition(cond_var1, cond_var2, 10);
    printf("Test6 result: %d, global_acc: %d\n", r6, global_acc);
    
    int r7 = test_nested_modification(cond_var1, cond_var2, 5);
    printf("Test7 result: %d, global_acc: %d\n", r7, global_acc);
    
    int r8 = test_safe_conversion(cond_var1, cond_var2);
    printf("Test8 result: %d, global_acc: %d\n", r8, global_acc);
    
    printf("Final global_acc: %d\n", global_acc);
    
    return global_acc != 0 ? 0 : 1;
}
