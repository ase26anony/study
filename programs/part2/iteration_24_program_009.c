#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization and track results */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Force runtime evaluation */
    if (global_accumulator > 0) {
        x = rand() % 100;
    }
    
    /* Critical pattern: condition variable x modified in then-block */
    if (x > 0) {  /* test_expr uses x */
        x = 10;    /* MODIFIES x in then-block - should fail if-conversion */
        result = y * 2;
        global_accumulator++;
    } else {
        result = y / 2;
        global_accumulator--;
    }
    
    /* Use x to prevent dead code elimination */
    return result + x;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
float test_safe_pattern(float a, float b) {
    float result = 0.0f;
    volatile float cond_var = a;  /* volatile to prevent optimization */
    
    /* Multiple if-statements to create larger basic block */
    if (cond_var > 0.0f) {  /* test_expr uses cond_var */
        result = b * 3.14f;  /* Does NOT modify cond_var */
        global_accumulator += 2;
    } else {
        result = b / 3.14f;
        global_accumulator -= 2;
    }
    
    /* Another if-statement with same condition variable */
    if (cond_var < 100.0f) {
        result += 1.0f;
    }
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int* ptr, int threshold) {
    int local_copy = *ptr;
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local_copy > threshold, 1)) {
        *ptr = threshold - 1;  /* Modifies through pointer - may affect condition */
        result = 100;
        global_accumulator *= 2;
    } else {
        result = 200;
        global_accumulator /= 2;
    }
    
    return result;
}

/* Test 4: Complex expression in condition with modification */
__attribute__((noinline, optimize("O3")))
int test_complex_condition(int a, int b, int c) {
    int cond_base = a + b;
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Complex condition expression */
        if ((cond_base * c) > (a * b)) {  /* test_expr uses cond_base */
            cond_base += i;  /* MODIFIES cond_base in then-block */
            result += c * i;
            global_cond = i;
        } else {
            result -= c * i;
            global_cond = -i;
        }
    }
    
    return result;
}

/* Test 5: Mixed safe/unsafe patterns in same function */
__attribute__((noinline, optimize("O2")))
void test_mixed_patterns(int* data, int size) {
    int local_counter = 0;
    
    for (int i = 0; i < size; i++) {
        /* Safe pattern first */
        if (data[i] > 0) {
            local_counter += data[i];
            global_accumulator++;
        } else {
            local_counter -= data[i];
            global_accumulator--;
        }
        
        /* Unsafe pattern - modifies condition source */
        if (local_counter > 100) {  /* test_expr uses local_counter */
            local_counter = 50;  /* MODIFIES local_counter in then-block */
            data[i] = 999;
        }
        
        /* Another safe pattern */
        if (global_cond > 0) {
            data[i] += local_counter;
        }
    }
}

/* Test 6: Volatile condition variable with modification */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int* vptr) {
    int result = 0;
    
    /* Force multiple evaluations */
    for (int attempt = 0; attempt < 2; attempt++) {
        if (*vptr > 0) {  /* test_expr uses volatile read */
            *vptr = *vptr - 1;  /* MODIFIES through volatile pointer */
            result += 50;
        } else {
            result += 25;
        }
    }
    
    return result;
}

/* Test 7: Function call that might modify condition variable */
extern int external_helper(int*);
__attribute__((noinline, optimize("O2")))
int test_function_call_condition(int base) {
    int result = 0;
    int cond_var = base;
    
    /* Condition variable potentially modified by function call */
    if (cond_var > 100) {
        cond_var = external_helper(&cond_var);  /* May modify cond_var */
        result = 1;
    } else {
        result = 0;
    }
    
    return result;
}

/* Dummy external function */
int external_helper(int* x) {
    *x = (*x) / 2;
    return *x;
}

/* Test 8: Nested if-statements with modification in inner block */
__attribute__((noinline, optimize("O3")))
int test_nested_modification(int a, int b) {
    int x = a;
    int y = b;
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            x = y * 2;  /* MODIFIES x in nested then-block */
            result = 100;
        } else {
            result = 50;
        }
        global_accumulator += result;
    } else {
        result = 10;
        global_accumulator -= result;
    }
    
    return result + x;
}

/* Main driver with runtime-controlled execution */
int main(int argc, char** argv) {
    int seed = 0;
    
    /* Use argv for runtime control to prevent compile-time optimization */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize test variables */
    int x = rand() % 200 - 100;  /* Range -100 to 100 */
    float f = (rand() % 1000) / 10.0f;
    int array[10];
    volatile int volatile_var = 50;
    
    for (int i = 0; i < 10; i++) {
        array[i] = rand() % 200 - 100;
    }
    
    printf("Starting if-conversion tests with seed %d\n", seed);
    printf("Initial global_accumulator: %d\n", global_accumulator);
    
    /* Execute all test patterns */
    int r1 = test_unsafe_modification(x, 42);
    printf("Test 1 result: %d, accumulator: %d\n", r1, global_accumulator);
    
    float r2 = test_safe_pattern(f, 2.0f);
    printf("Test 2 result: %.2f, accumulator: %d\n", r2, global_accumulator);
    
    int ptr_val = rand() % 100;
    int r3 = test_pointer_condition(&ptr_val, 50);
    printf("Test 3 result: %d, ptr_val: %d, accumulator: %d\n", r3, ptr_val, global_accumulator);
    
    int r4 = test_complex_condition(x, 20, 3);
    printf("Test 4 result: %d, global_cond: %d\n", r4, global_cond);
    
    test_mixed_patterns(array, 10);
    printf("Test 5 completed, array[0]: %d, accumulator: %d\n", array[0], global_accumulator);
    
    int r6 = test_volatile_condition(&volatile_var);
    printf("Test 6 result: %d, volatile_var: %d\n", r6, volatile_var);
    
    int r7 = test_function_call_condition(x);
    printf("Test 7 result: %d\n", r7);
    
    int r8 = test_nested_modification(x, 30);
    printf("Test 8 result: %d\n", r8);
    
    /* Final summary */
    printf("\nFinal global_accumulator: %d\n", global_accumulator);
    printf("All tests completed.\n");
    
    return global_accumulator != 0 ? 0 : 1;
}
