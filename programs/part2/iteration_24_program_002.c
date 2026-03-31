#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification of condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in then-block */
    if (x > 0) {
        x = y * 2;  /* MODIFIES condition variable */
        result = x + 10;
        global_accumulator += result;
    } else {
        result = y - 5;
        global_accumulator += result;
    }
    
    /* Use result to prevent dead code elimination */
    return result + (x & 1);
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int temp = a;
    
    /* Condition variable a is NOT modified in then-block */
    if (a > 100) {
        temp = b * 3;  /* Modifies temp, not a */
        global_accumulator += temp;
        return temp;
    } else {
        temp = b / 2;
        global_accumulator += temp;
        return temp;
    }
}

/* Test 3: Pointer dereference condition with modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int local_copy = *ptr;
    
    /* Condition uses pointer dereference, modified in then-block */
    if (*ptr > threshold) {
        *ptr = threshold - 1;  /* MODIFIES condition expression */
        global_accumulator += *ptr;
        return 1;
    } else {
        global_accumulator += local_copy;
        return 0;
    }
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float g) {
    volatile float vf = f;  /* volatile to prevent optimization */
    
    /* Float condition variable modified in then-block */
    if (vf > 0.0f) {
        vf = g * 2.0f;  /* MODIFIES condition variable */
        global_accumulator += (int)vf;
        return vf;
    } else {
        global_accumulator += (int)g;
        return g;
    }
}

/* Test 5: Complex expression in condition */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int a, int b, int c) {
    int cond = a + b;
    
    /* Complex condition with modification */
    if ((a + b) > c) {
        a = b * c;  /* Modifies part of condition expression (a) */
        global_accumulator += a;
        return a;
    } else {
        global_accumulator += cond;
        return cond;
    }
}

/* Test 6: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O3")))
int test_loop_with_branch(int iterations, int seed) {
    int sum = 0;
    int cond_var = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* This branch should be considered for if-conversion */
        if (cond_var > i) {
            cond_var--;  /* MODIFIES condition variable inside loop */
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Add some computation to make if-conversion beneficial */
        cond_var += (i % 3);
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 7: Multiple related if-statements */
__attribute__((noinline, optimize("O2")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First branch - unsafe */
    if (x > 0) {
        x = y;  /* Modifies condition variable */
        result += 1;
    } else {
        result -= 1;
    }
    
    /* Second branch - safe */
    if (y > 0) {
        result += 2;  /* Doesn't modify y */
    } else {
        result -= 2;
    }
    
    /* Third branch with __builtin_expect */
    if (__builtin_expect(z > 0, 1)) {
        z = x + y;  /* Modifies condition variable */
        result += 3;
    } else {
        result -= 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 8: Volatile condition variable */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int* vptr) {
    int local = *vptr;
    
    if (*vptr > 100) {
        *vptr = 50;  /* Modifies volatile condition */
        global_accumulator += *vptr;
        return 1;
    }
    
    global_accumulator += local;
    return 0;
}

/* Test 9: Nested if with modification */
__attribute__((noinline, optimize("O2")))
int test_nested_branch(int a, int b) {
    int result = 0;
    
    if (a > 10) {
        if (b > 5) {
            a = b * 2;  /* Modifies outer condition variable */
            result = 1;
        } else {
            result = 2;
        }
        global_accumulator += result + a;
    } else {
        result = 3;
        global_accumulator += result;
    }
    
    return result;
}

/* Test 10: Function call that might modify condition */
extern int external_func(int*);
__attribute__((noinline, optimize("O3")))
int test_function_call_condition(int* ptr) {
    int local = *ptr;
    
    if (*ptr > 0) {
        external_func(ptr);  /* May modify *ptr through pointer */
        global_accumulator += *ptr;
        return 1;
    }
    
    global_accumulator += local;
    return 0;
}

/* Dummy external function */
int external_func(int* p) {
    *p = *p / 2;
    return *p;
}

int main(int argc, char** argv) {
    /* Use argv to create runtime-dependent values */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    int cond_var2 = rand() % 100;
    float cond_var3 = (float)(rand() % 100) / 10.0f;
    volatile int cond_var4 = rand() % 100;
    int array[5] = {rand() % 100, rand() % 100, rand() % 100, 
                    rand() % 100, rand() % 100};
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    printf("Initial accumulator: %d\n", global_accumulator);
    
    /* Call test functions with runtime-dependent parameters */
    int result1 = test_unsafe_modification(cond_var1, cond_var2);
    printf("Test1 result: %d, Accumulator: %d\n", result1, global_accumulator);
    
    int result2 = test_safe_pattern(cond_var1, cond_var2);
    printf("Test2 result: %d, Accumulator: %d\n", result2, global_accumulator);
    
    int result3 = test_pointer_condition(&array[0], 50);
    printf("Test3 result: %d, Accumulator: %d\n", result3, global_accumulator);
    
    float result4 = test_float_condition(cond_var3, cond_var3 * 2.0f);
    printf("Test4 result: %.2f, Accumulator: %d\n", result4, global_accumulator);
    
    int result5 = test_complex_condition(cond_var1, cond_var2, 75);
    printf("Test5 result: %d, Accumulator: %d\n", result5, global_accumulator);
    
    int result6 = test_loop_with_branch(10, seed % 20);
    printf("Test6 result: %d, Accumulator: %d\n", result6, global_accumulator);
    
    int result7 = test_multiple_branches(cond_var1, cond_var2, seed % 50);
    printf("Test7 result: %d, Accumulator: %d\n", result7, global_accumulator);
    
    int result8 = test_volatile_condition(&cond_var4);
    printf("Test8 result: %d, Accumulator: %d\n", result8, global_accumulator);
    
    int result9 = test_nested_branch(cond_var1, cond_var2);
    printf("Test9 result: %d, Accumulator: %d\n", result9, global_accumulator);
    
    int result10 = test_function_call_condition(&array[1]);
    printf("Test10 result: %d, Accumulator: %d\n", result10, global_accumulator);
    
    printf("Final accumulator value: %d\n", global_accumulator);
    
    return global_accumulator > 0 ? 0 : 1;
}
