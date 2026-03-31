#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - condition variable modified in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable */
        result = y * 2;
        global_accumulator += result;
    } else {
        result = y / 2;
        global_accumulator -= result;
    }
    
    /* Use x to prevent dead code elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    int temp = x;  /* Copy to avoid modifying original */
    
    if (temp > 0) {
        /* temp is NOT the same as x, so condition variable x is safe */
        result = y * 3;
        global_accumulator += result;
    } else {
        result = y / 3;
        global_accumulator -= result;
    }
    
    return result + x;  /* x unchanged */
}

/* Test 3: Pointer dereference condition with modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int y) {
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > 0) {
        *ptr = 0;  /* MODIFIES the dereferenced value */
        result = y * 4;
        global_accumulator += result;
    } else {
        result = y / 4;
        global_accumulator -= result;
    }
    
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float x, float y) {
    float result = 0.0f;
    
    if (x > 0.0f) {
        x = x * 2.0f;  /* MODIFIES condition variable */
        result = y * 1.5f;
        global_accumulator += (int)result;
    } else {
        result = y / 1.5f;
        global_accumulator -= (int)result;
    }
    
    return result + x;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *v, int y) {
    int result = 0;
    
    if (*v > 0) {
        *v = *v - 1;  /* MODIFIES volatile condition */
        result = y * 5;
        global_accumulator += result;
    } else {
        result = y / 5;
        global_accumulator -= result;
    }
    
    return result;
}

/* Test 6: Complex expression in condition */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int x, int y, int z) {
    int result = 0;
    int cond = x + y;
    
    if (cond > z) {
        x = x + y;  /* MODIFIES part of original condition expression */
        result = z * 6;
        global_accumulator += result;
    } else {
        result = z / 6;
        global_accumulator -= result;
    }
    
    return result + x;
}

/* Test 7: Nested if with modification in inner block */
__attribute__((noinline, optimize("O2")))
int test_nested_modification(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            x = 20;  /* MODIFIES outer condition variable */
            result = z * 7;
            global_accumulator += result;
        } else {
            result = z / 7;
            global_accumulator -= result;
        }
    } else {
        result = z;
        global_accumulator += result;
    }
    
    return result + x;
}

/* Test 8: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O3")))
int test_loop_with_branch(int iterations, int seed) {
    int sum = 0;
    int cond_var = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* This branch inside loop is if-conversion candidate */
        if (cond_var > i) {
            cond_var--;  /* MODIFIES condition variable */
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Add some computation to make if-conversion attractive */
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 9: Multiple related if-statements */
__attribute__((noinline, optimize("O2")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First if - safe */
    if (x > 0) {
        result += y;
    } else {
        result -= y;
    }
    
    /* Second if - modifies condition variable used in third if */
    int cond = z;
    if (y > 0) {
        cond = y;  /* Modifies variable used in next condition */
    }
    
    /* Third if - uses modified variable */
    if (cond > 0) {
        x = 30;  /* MODIFIES variable from first condition */
        result += z * 2;
    } else {
        result += z;
    }
    
    global_accumulator += result;
    return result + x;
}

/* Test 10: Using __builtin_expect to influence branch prediction */
__attribute__((noinline, optimize("O3")))
int test_builtin_expect(int x, int y) {
    int result = 0;
    
    if (__builtin_expect(x > 0, 1)) {
        x = 40;  /* MODIFIES condition variable */
        result = y * 10;
        global_accumulator += result;
    } else {
        result = y / 10;
        global_accumulator -= result;
    }
    
    return result + x;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line or random seed to prevent constant folding */
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
    int cond_var5 = rand() % 100;
    int *ptr_var = &cond_var5;
    
    int result = 0;
    
    /* Call test functions with runtime-determined values */
    result += test_unsafe_modification(cond_var1, rand() % 50 + 1);
    result += test_safe_pattern(cond_var2, rand() % 50 + 1);
    result += test_pointer_condition(ptr_var, rand() % 50 + 1);
    result += (int)test_float_condition(cond_var3, (float)(rand() % 50 + 1));
    result += test_volatile_condition(&cond_var4, rand() % 50 + 1);
    result += test_complex_condition(cond_var1, cond_var2, rand() % 50 + 1);
    result += test_nested_modification(cond_var1, cond_var2, rand() % 50 + 1);
    result += test_loop_with_branch(10 + (rand() % 20), rand() % 50);
    result += test_multiple_branches(cond_var1, cond_var2, rand() % 50 + 1);
    result += test_builtin_expect(cond_var1, rand() % 50 + 1);
    
    /* Print results to ensure code isn't eliminated */
    printf("Final accumulator: %d\n", global_accumulator);
    printf("Result: %d\n", result);
    printf("Seed used: %d\n", seed);
    
    return result != 0 ? 0 : 1;
}
