#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then-block */
    if (x > 0) {
        /* This modification should trigger the safety check */
        x = y + 5;  // Modifies the condition variable
        result = x * 2;
    } else {
        result = y * 3;
    }
    
    /* Use result to prevent dead code elimination */
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    
    /* Condition variable x is NOT modified in the then-block */
    if (x > 0) {
        /* Safe: only modifies local variable */
        result = y * 2;
    } else {
        result = y * 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int y) {
    int result = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > 0) {
        /* Unsafe: modifies the memory location used in condition */
        *ptr = y + 10;  // Modifies the condition expression
        result = *ptr * 2;
    } else {
        result = y * 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 4: Float condition with unsafe modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float x, float y) {
    float result = 0.0f;
    
    /* Float condition */
    if (x > 0.0f) {
        /* Unsafe: modifies the float condition variable */
        x = y * 2.0f;  // Modifies the condition variable
        result = x + 1.0f;
    } else {
        result = y - 1.0f;
    }
    
    global_accumulator += (int)result;
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *vptr, int y) {
    int result = 0;
    
    /* Condition uses volatile variable */
    if (*vptr > 0) {
        /* Unsafe: modifies volatile condition variable */
        *vptr = y + 20;  // Modifies the condition expression
        result = *vptr * 2;
    } else {
        result = y * 3;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 6: Complex expression in condition with unsafe modification */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int x, int y, int z) {
    int result = 0;
    
    /* Complex condition expression */
    if ((x + y) > z) {
        /* Unsafe: modifies part of the condition expression (x) */
        x = z * 2;  // Modifies variable used in condition
        result = x + y;
    } else {
        result = y + z;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O2")))
int test_loop_with_branch(int iterations) {
    int sum = 0;
    int cond_var = 100;
    
    /* Loop to encourage if-conversion analysis */
    for (int i = 0; i < iterations; i++) {
        /* Branch inside loop - good candidate for if-conversion */
        if (cond_var > 50) {
            /* Unsafe: modifies condition variable */
            cond_var = i + 30;  // Modifies the condition variable
            sum += cond_var * 2;
        } else {
            sum += i * 3;
        }
        
        /* Mix with other operations to create larger basic block */
        sum += (i % 2 == 0) ? i : -i;
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 8: Multiple related if-statements */
__attribute__((noinline, optimize("O3")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    
    /* First branch */
    if (x > 0) {
        /* Unsafe: modifies condition variable */
        x = y + 1;  // Modifies x used in next condition
        result += 10;
    } else {
        result += 5;
    }
    
    /* Second branch using same variable */
    if (x < 10) {
        result += 20;
    } else {
        result += 15;
    }
    
    /* Third branch with __builtin_expect */
    if (__builtin_expect(y > 0, 1)) {
        result += 30;
    } else {
        result += 25;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 9: Safe pattern with arithmetic in both branches */
__attribute__((noinline, optimize("O2")))
int test_safe_arithmetic(int x, int y) {
    int result = 0;
    
    /* Safe: condition variable not modified */
    if (x > y) {
        result = (x * 2) + (y / 2);
    } else {
        result = (y * 2) + (x / 2);
    }
    
    global_accumulator += result;
    return result;
}

/* Test 10: Function call that might modify condition */
extern int external_func(int*);
__attribute__((noinline, optimize("O3")))
int test_function_call_condition(int x, int y) {
    int result = 0;
    int local_x = x;
    
    /* Condition variable passed to function */
    if (local_x > 0) {
        /* Function call might modify local_x through pointer */
        result = external_func(&local_x);  // Potentially modifies condition variable
        result += 10;
    } else {
        result = y * 2;
    }
    
    global_accumulator += result;
    return result;
}

/* Dummy external function */
int external_func(int *ptr) {
    *ptr += 5;  // Modifies the variable
    return *ptr * 2;
}

int main(int argc, char *argv[]) {
    int seed;
    
    /* Use command line argument or time for randomness */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize variables with random values to prevent constant folding */
    int x1 = rand() % 100;
    int x2 = rand() % 100;
    int x3 = rand() % 100;
    int y1 = rand() % 100;
    int y2 = rand() % 100;
    int y3 = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    
    volatile int volatile_var = rand() % 100;
    int ptr_val = rand() % 100;
    int *ptr = &ptr_val;
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    
    /* Execute all test functions */
    int r1 = test_unsafe_modification(x1, y1);
    int r2 = test_safe_pattern(x2, y2);
    int r3 = test_pointer_condition(ptr, y3);
    float r4 = test_float_condition(f1, f2);
    int r5 = test_volatile_condition(&volatile_var, x3);
    int r6 = test_complex_condition(x1, x2, x3);
    int r7 = test_loop_with_branch(10 + (rand() % 20));
    int r8 = test_multiple_branches(x1, x2, x3);
    int r9 = test_safe_arithmetic(x2, y2);
    int r10 = test_function_call_condition(x3, y3);
    
    printf("Results: %d, %d, %d, %.2f, %d, %d, %d, %d, %d, %d\n",
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
