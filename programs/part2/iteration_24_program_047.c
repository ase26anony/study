#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;
int global_cond = 0;

/* Function to create observable side effects */
void side_effect(int value) {
    global_counter += value;
}

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then block */
    if (x > 0) {
        x = 10;  /* Direct modification of condition variable */
        result = y * 2;
        side_effect(1);
    } else {
        result = y / 2;
        side_effect(2);
    }
    
    /* Use x to prevent dead store elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int a, int b) {
    int result = 0;
    int cond = a;
    
    /* cond is not modified in the then block */
    if (cond < 100) {
        result = b + 42;
        side_effect(3);
    } else {
        result = b - 42;
        side_effect(4);
    }
    
    return result;
}

/* Test 3: Pointer dereference condition with modification */
__attribute__((optimize("O2"), noinline))
int test_pointer_condition(int *ptr, int val) {
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > 0) {
        *ptr = val;  /* Modifies the memory location used in condition */
        result = val * 3;
        side_effect(5);
    } else {
        result = val + 7;
        side_effect(6);
    }
    
    return result;
}

/* Test 4: Volatile condition variable */
__attribute__((optimize("O3"), noinline))
int test_volatile_condition(volatile int *vptr, int x) {
    int result = 0;
    
    if (*vptr != 0) {
        *vptr = x;  /* Modifies volatile condition variable */
        result = x * x;
        side_effect(7);
    } else {
        result = x + x;
        side_effect(8);
    }
    
    return result;
}

/* Test 5: Nested if-statements with condition modification */
__attribute__((optimize("O2"), noinline))
int test_nested_ifs(int a, int b, int c) {
    int result = 0;
    int cond1 = a;
    int cond2 = b;
    
    /* First if - safe */
    if (cond1 > 0) {
        result = b + c;
        side_effect(9);
    } else {
        result = b - c;
        side_effect(10);
    }
    
    /* Second if - unsafe (modifies condition variable) */
    if (cond2 < 100) {
        cond2 = 200;  /* Modifies condition variable */
        result += a * 2;
        side_effect(11);
    } else {
        result += a / 2;
        side_effect(12);
    }
    
    return result + cond2;
}

/* Test 6: Loop with if-conversion candidate */
__attribute__((optimize("O3"), noinline))
int test_loop_with_branch(int n, int *data) {
    int sum = 0;
    int local_cond = global_cond;
    
    for (int i = 0; i < n; i++) {
        /* This if may be if-converted */
        if (local_cond > i) {
            data[i] = i * 2;  /* Doesn't modify local_cond */
            sum += i;
            side_effect(13);
        } else {
            data[i] = i / 2;
            sum -= i;
            side_effect(14);
        }
        
        /* Modify condition variable outside the if block */
        if (i % 3 == 0) {
            local_cond++;
        }
    }
    
    return sum;
}

/* Test 7: Builtin expect with unsafe modification */
__attribute__((optimize("O2"), noinline))
int test_builtin_expect_unsafe(int x, int y) {
    int result = 0;
    int cond = x;
    
    /* Hint that condition is likely true */
    if (__builtin_expect(cond > 50, 1)) {
        cond = y;  /* Modifies condition variable */
        result = y * 3;
        side_effect(15);
    } else {
        result = x * 4;
        side_effect(16);
    }
    
    return result + cond;
}

/* Test 8: Multiple related if-statements */
__attribute__((optimize("O3"), noinline))
int test_multiple_ifs(int a, int b, int c) {
    int result = 0;
    int cond = a;
    
    /* Sequence of if-statements */
    if (cond > 0) {
        result = b;
        side_effect(17);
    }
    
    if (cond < 100) {
        cond = c;  /* Modifies condition variable for next if */
        result += c;
        side_effect(18);
    }
    
    if (cond == c) {  /* This condition depends on previous modification */
        result *= 2;
        side_effect(19);
    }
    
    return result;
}

/* Test 9: Float condition with modification */
__attribute__((optimize("O2"), noinline))
float test_float_condition(float f, float g) {
    float result = 0.0f;
    
    if (f > 0.0f) {
        f = g;  /* Modifies float condition variable */
        result = g * 2.0f;
        side_effect(20);
    } else {
        result = g / 2.0f;
        side_effect(21);
    }
    
    return result + f;
}

/* Test 10: Complex expression in condition */
__attribute__((optimize("O3"), noinline))
int test_complex_condition(int a, int b, int c) {
    int result = 0;
    int x = a;
    int y = b;
    
    /* Complex condition expression */
    if ((x * y) > (a + b + c)) {
        x = c;  /* Modifies part of the condition expression */
        result = (a + b) * c;
        side_effect(22);
    } else {
        result = (a - b) * c;
        side_effect(23);
    }
    
    return result + x;
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
    
    /* Initialize test variables */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 100) / 10.0f;
    float g = (float)(rand() % 100) / 10.0f;
    
    int ptr_val = rand() % 100;
    int *ptr = &ptr_val;
    
    volatile int volatile_val = rand() % 100;
    volatile int *vptr = &volatile_val;
    
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    global_cond = rand() % 50;
    
    /* Execute all test functions */
    int result = 0;
    
    result += test_unsafe_modification(x, y);
    result += test_safe_pattern(y, z);
    result += test_pointer_condition(ptr, x);
    result += test_volatile_condition(vptr, y);
    result += test_nested_ifs(x, y, z);
    result += test_loop_with_branch(50, data);
    result += test_builtin_expect_unsafe(x, z);
    result += test_multiple_ifs(x, y, z);
    result += (int)test_float_condition(f, g);
    result += test_complex_condition(x, y, z);
    
    /* Make results observable */
    global_result = result;
    
    printf("Seed: %d\n", seed);
    printf("Global counter: %d\n", global_counter);
    printf("Result: %d\n", result);
    printf("Global result: %d\n", global_result);
    
    /* Use results to prevent optimization */
    if (global_result > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
