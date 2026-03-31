#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables for condition testing */
int global_cond = 0;
volatile int volatile_cond = 0;
int *global_ptr = NULL;

/* Accumulators to prevent optimization */
int global_acc = 0;
int results[10] = {0};

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable that will be modified in then-block */
    int cond_var = x;
    
    /* Use argv/rand to prevent constant folding */
    if (cond_var > 0) {  /* test_expr: cond_var > 0 */
        /* UNSAFE: Modifies the condition variable */
        cond_var = y;    /* This should trigger modified_in_p check */
        result = x * 2;
        global_acc += result;
    } else {
        result = x / 2;
        global_acc -= result;
    }
    
    /* Use result to prevent dead code elimination */
    return result + cond_var;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    int cond_var = x;
    
    if (cond_var > 100) {  /* test_expr: cond_var > 100 */
        /* SAFE: Does not modify cond_var */
        result = y * 3;
        global_acc += result * 2;
    } else {
        result = y / 3;
        global_acc -= result * 2;
    }
    
    /* cond_var unchanged in then-block, if-conversion should proceed */
    return result + cond_var;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int result = 0;
    
    if (*ptr > threshold) {  /* test_expr: *ptr > threshold */
        /* UNSAFE: Modifies through pointer dereference */
        *ptr = threshold - 1;  /* This modifies the memory read in condition */
        result = 100;
        global_acc += 100;
    } else {
        *ptr = threshold + 1;
        result = 50;
        global_acc += 50;
    }
    
    return result;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float x, float y) {
    float result = 0.0f;
    float cond_var = x;
    
    /* Prevent constant folding with runtime value */
    if (cond_var > 0.5f) {  /* test_expr: cond_var > 0.5f */
        /* UNSAFE: Modifies float condition variable */
        cond_var = y * 2.0f;
        result = x * 1.5f;
        global_acc += (int)result;
    } else {
        cond_var = y / 2.0f;
        result = x * 0.5f;
        global_acc -= (int)result;
    }
    
    return result + cond_var;
}

/* Test 5: Global variable condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_global_condition(int x) {
    int result = 0;
    
    if (global_cond > x) {  /* test_expr: global_cond > x */
        /* UNSAFE: Modifies global used in condition */
        global_cond = x - 1;
        result = 200;
        global_acc += 200;
    } else {
        global_cond = x + 1;
        result = 100;
        global_acc += 100;
    }
    
    return result;
}

/* Test 6: Volatile variable condition */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(int x) {
    int result = 0;
    
    /* Volatile read in condition */
    if (volatile_cond > x) {  /* test_expr: volatile_cond > x */
        /* Even if we modify it, volatile has side effects */
        volatile_cond = x;
        result = 300;
        global_acc += 300;
    } else {
        volatile_cond = x + 10;
        result = 150;
        global_acc += 150;
    }
    
    return result;
}

/* Test 7: Complex expression in condition with partial modification */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int x, int y, int z) {
    int result = 0;
    int a = x, b = y, c = z;
    
    /* Complex condition expression */
    if ((a + b) > (c * 2)) {  /* test_expr: (a + b) > (c * 2) */
        /* Modifies 'a' which is part of condition expression */
        a = y * 2;  /* This should trigger modified_in_p */
        result = a + b + c;
        global_acc += result;
    } else {
        b = x * 2;
        result = a + b - c;
        global_acc -= result;
    }
    
    return result;
}

/* Test 8: Nested if-statements with modification in inner block */
__attribute__((noinline, optimize("O3")))
int test_nested_ifs(int x, int y, int z) {
    int result = 0;
    int cond_var = x;
    
    if (cond_var > 0) {  /* Outer condition */
        if (y > 0) {     /* Inner condition */
            /* Modifies outer condition variable */
            cond_var = z;  /* This should be detected */
            result = x + y + z;
            global_acc += result;
        } else {
            result = x - y - z;
            global_acc -= result;
        }
    } else {
        result = y * z;
        global_acc += result * 2;
    }
    
    return result + cond_var;
}

/* Test 9: Loop with if-conversion candidate */
__attribute__((noinline, optimize("O2")))
int test_loop_with_branch(int n, int *data) {
    int sum = 0;
    int cond_var = 0;
    
    for (int i = 0; i < n; i++) {
        cond_var = data[i];
        
        /* Branch inside loop - good if-conversion candidate */
        if (cond_var > 50) {  /* test_expr: cond_var > 50 */
            /* UNSAFE in some iterations */
            if (i % 3 == 0) {
                cond_var = 25;  /* Modifies condition variable */
            }
            sum += cond_var * 2;
        } else {
            sum += cond_var;
        }
    }
    
    global_acc += sum;
    return sum;
}

/* Test 10: Multiple related if-statements in sequence */
__attribute__((noinline, optimize("O3")))
int test_multiple_branches(int x, int y, int z) {
    int result = 0;
    int cond1 = x, cond2 = y;
    
    /* First if-statement */
    if (cond1 > 10) {  /* test_expr: cond1 > 10 */
        cond1 = y;     /* Modification */
        result += 5;
    } else {
        result += 2;
    }
    
    /* Second if-statement */
    if (cond2 < 20) {  /* Different condition variable */
        result += 3;
        /* cond2 not modified - safe */
    } else {
        cond2 = x;     /* Modification in else-block */
        result += 1;
    }
    
    /* Third if-statement with __builtin_expect */
    if (__builtin_expect((cond1 + cond2) > 30, 0)) {
        cond1 = z;     /* Modification */
        result *= 2;
    }
    
    global_acc += result;
    return result + cond1 + cond2;
}

int main(int argc, char *argv[]) {
    /* Initialize random seed from argv or time */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test data */
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = rand() % 100;
    }
    
    global_ptr = &global_cond;
    
    /* Run tests with varying inputs to explore different paths */
    results[0] = test_unsafe_modification(rand() % 100, rand() % 100);
    results[1] = test_safe_pattern(rand() % 200, rand() % 200);
    
    int ptr_val = rand() % 100;
    results[2] = test_pointer_condition(&ptr_val, 50);
    
    results[3] = (int)test_float_condition((float)(rand() % 100) / 100.0f, 
                                          (float)(rand() % 100) / 100.0f);
    
    global_cond = rand() % 100;
    results[4] = test_global_condition(rand() % 100);
    
    volatile_cond = rand() % 100;
    results[5] = test_volatile_condition(rand() % 100);
    
    results[6] = test_complex_condition(rand() % 100, rand() % 100, rand() % 100);
    results[7] = test_nested_ifs(rand() % 100, rand() % 100, rand() % 100);
    results[8] = test_loop_with_branch(50, data);
    results[9] = test_multiple_branches(rand() % 100, rand() % 100, rand() % 100);
    
    /* Print results to prevent optimization */
    printf("Seed: %d\n", seed);
    printf("Global accumulator: %d\n", global_acc);
    printf("Results: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
