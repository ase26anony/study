#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_accumulator = 0;
float global_float = 0.0f;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in then-block */
    if (x > 0) {
        x = y * 2;  /* MODIFIES condition variable */
        result = x + 10;
        global_counter++;
    } else {
        result = y - 5;
    }
    
    /* Use result to prevent dead code elimination */
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int a, int b) {
    int local = a;
    int result = 0;
    
    /* Condition variable 'a' is NOT modified in then-block */
    if (a > b) {
        local = b * 3;  /* Modifies different variable */
        result = local + a;
        global_counter += 2;
    } else {
        result = a + b;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > threshold) {
        (*ptr)++;  /* MODIFIES the dereferenced value used in condition */
        result = *ptr * 2;
        global_counter += 3;
    } else {
        result = threshold - *ptr;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((optimize("O3"), noinline))
float test_float_condition(float f, float g) {
    float result = 0.0f;
    
    /* Float condition variable modified in then-block */
    if (f > 1.0f) {
        f = g * 2.0f;  /* MODIFIES condition variable */
        result = f + 3.14f;
        global_float += result;
    } else {
        result = g - f;
    }
    
    return result;
}

/* Test 5: Volatile condition variable */
__attribute__((optimize("O2"), noinline))
int test_volatile_condition(volatile int *v) {
    int result = 0;
    
    /* Volatile access in condition */
    if (*v > 100) {
        *v = 50;  /* MODIFIES volatile variable used in condition */
        result = 1;
        global_counter += 4;
    } else {
        result = 0;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 6: Complex expression in condition with partial modification */
__attribute__((optimize("O3"), noinline))
int test_complex_condition(int a, int b, int c) {
    int x = a;
    int y = b;
    int result = 0;
    
    /* Complex condition expression */
    if ((x + y) > c) {
        x = c;  /* Modifies part of the condition expression (x) */
        result = x * y;
        global_counter += 5;
    } else {
        result = c - (x + y);
    }
    
    global_accumulator += result;
    return result;
}

/* Test 7: Loop with if-conversion candidate */
__attribute__((optimize("O2"), noinline))
int test_loop_with_branch(int iterations, int seed) {
    int sum = 0;
    int cond_var = seed;
    
    /* Loop to encourage if-conversion analysis */
    for (int i = 0; i < iterations; i++) {
        /* Branch hint to influence if-conversion heuristics */
        if (__builtin_expect((cond_var % 3) > 0, 1)) {
            cond_var += i;  /* MODIFIES condition variable in loop */
            sum += cond_var * 2;
        } else {
            sum += i;
        }
        
        /* Additional operation to create larger basic block */
        cond_var = (cond_var * 1103515245 + 12345) & 0x7fffffff;
    }
    
    global_accumulator += sum;
    return sum;
}

/* Test 8: Nested if-statements */
__attribute__((optimize("O3"), noinline))
int test_nested_branches(int a, int b, int c) {
    int result = 0;
    int temp = a;
    
    /* First level */
    if (temp > b) {
        /* Second level - modifies condition variable of outer branch */
        if (b > c) {
            temp = c;  /* MODIFIES variable used in outer condition */
            result = temp * 3;
        } else {
            result = b * 2;
        }
        global_counter += 6;
    } else {
        if (c > a) {
            result = c - a;
        } else {
            result = a - c;
        }
    }
    
    global_accumulator += result;
    return result;
}

/* Test 9: Multiple related if-statements in sequence */
__attribute__((optimize("O2"), noinline))
int test_sequence_of_branches(int x, int y, int z) {
    int result = 0;
    
    /* First if - safe */
    if (x > 0) {
        result += x;
        y = z;  /* Modifies variable used in next condition */
    } else {
        result -= x;
    }
    
    /* Second if - potentially unsafe due to y modification above */
    if (y > z) {
        y = x;  /* MODIFIES condition variable */
        result += y * 2;
        global_counter += 7;
    } else {
        result += z;
    }
    
    /* Third if - safe */
    if (z > 0) {
        result *= 2;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 10: Function call that might modify condition variable */
__attribute__((optimize("O3"), noinline))
int modify_value(int *val) {
    *val += 10;
    return *val;
}

__attribute__((optimize("O2"), noinline))
int test_function_call_condition(int a, int b) {
    int local = a;
    int result = 0;
    
    if (local > b) {
        /* Function call modifies the condition variable */
        result = modify_value(&local);
        global_counter += 8;
    } else {
        result = b - local;
    }
    
    global_accumulator += result;
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
    
    /* Initialize test variables with runtime-dependent values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 100) / 10.0f;
    float g = (float)(rand() % 100) / 10.0f;
    volatile int volatile_var = rand() % 200;
    int ptr_val = rand() % 150;
    int *ptr = &ptr_val;
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    printf("Initial values: x=%d, y=%d, z=%d, f=%.2f, g=%.2f, volatile_var=%d\n",
           x, y, z, f, g, volatile_var);
    
    /* Execute all test functions */
    int r1 = test_unsafe_modification(x, y);
    int r2 = test_safe_pattern(y, z);
    int r3 = test_pointer_condition(ptr, 50);
    float r4 = test_float_condition(f, g);
    int r5 = test_volatile_condition(&volatile_var);
    int r6 = test_complex_condition(x, y, z);
    int r7 = test_loop_with_branch(10, seed % 100);
    int r8 = test_nested_branches(x, y, z);
    int r9 = test_sequence_of_branches(x, y, z);
    int r10 = test_function_call_condition(y, z);
    
    printf("\nTest results:\n");
    printf("  test_unsafe_modification: %d\n", r1);
    printf("  test_safe_pattern: %d\n", r2);
    printf("  test_pointer_condition: %d\n", r3);
    printf("  test_float_condition: %.2f\n", r4);
    printf("  test_volatile_condition: %d\n", r5);
    printf("  test_complex_condition: %d\n", r6);
    printf("  test_loop_with_branch: %d\n", r7);
    printf("  test_nested_branches: %d\n", r8);
    printf("  test_sequence_of_branches: %d\n", r9);
    printf("  test_function_call_condition: %d\n", r10);
    
    printf("\nGlobal accumulator: %d\n", global_accumulator);
    printf("Global counter: %d\n", global_counter);
    printf("Global float: %.2f\n", global_float);
    
    /* Return non-deterministic result to prevent optimization */
    return (global_accumulator + global_counter) % 256;
}
