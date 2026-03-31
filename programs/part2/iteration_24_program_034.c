#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_modify = 0;
int global_result = 0;

/* Function 1: Unsafe pattern - modifies condition variable in then-block */
__attribute__((optimize("O2"), noinline))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in the then-block */
    if (x > 0) {  /* test_expr involves x */
        x = y * 2;  /* MODIFIES condition variable x */
        result = x + 10;
        global_counter++;
    } else {
        result = x - 5;
    }
    
    /* Use result to prevent dead code elimination */
    global_result ^= result;
    return result;
}

/* Function 2: Safe pattern - does NOT modify condition variable */
__attribute__((optimize("O3"), noinline))
int test_safe_pattern(int a, int b) {
    int local = a;
    
    /* Condition variable a is NOT modified in then-block */
    if (a < b) {
        local = b * 3;  /* Modifies local, not a */
        global_counter += 2;
    } else {
        local = a * 2;
    }
    
    /* Complex enough to encourage if-conversion */
    int temp = (local > 0) ? local : -local;
    global_result += temp;
    return temp;
}

/* Function 3: Pointer-based condition with unsafe modification */
__attribute__((optimize("O2"), noinline))
void test_pointer_condition(int *ptr, int threshold) {
    volatile int local_copy = *ptr;
    
    /* Condition depends on pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold - 1;  /* MODIFIES the memory condition depends on */
        global_modify++;
    } else {
        *ptr = threshold + 1;
    }
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local_copy > 100, 0)) {
        global_result -= 1;
    }
}

/* Function 4: Float condition with modification */
__attribute__((optimize("O3"), noinline))
float test_float_condition(float f, float g) {
    float result = 0.0f;
    
    /* Float condition variable */
    if (f > g) {
        f = g * 2.0f;  /* MODIFIES condition variable f */
        result = f + 1.5f;
        global_counter += 3;
    } else {
        result = g - f;
    }
    
    global_result += (int)result;
    return result;
}

/* Function 5: Multiple related if-statements in sequence */
__attribute__((optimize("O2"), noinline))
int test_multiple_ifs(int x, int y, int z) {
    int a = x, b = y, c = z;
    
    /* First if: safe */
    if (a > b) {
        c = a + b;
    } else {
        c = a - b;
    }
    
    /* Second if: UNSAFE - modifies condition variable */
    if (b < c) {
        b = c * 2;  /* MODIFIES b used in next condition */
        global_counter++;
    }
    
    /* Third if: depends on potentially modified b */
    if (b > a) {
        a = b / 2;
    }
    
    global_result += a + b + c;
    return a + b + c;
}

/* Function 6: Loop with if-conversion candidate */
__attribute__((optimize("O3"), noinline))
int test_loop_with_branch(int iterations, int seed) {
    int sum = 0;
    volatile int cond_var = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* This if should be considered for if-conversion */
        if (cond_var > i) {
            cond_var = i * 2;  /* MODIFIES condition variable in loop */
            sum += i;
            global_counter++;
        } else {
            sum -= i;
        }
        
        /* Prevent loop unrolling from eliminating branch */
        if (i % 7 == 0) {
            cond_var += 3;
        }
    }
    
    global_result += sum;
    return sum;
}

/* Function 7: Safe pattern with complex expression */
__attribute__((optimize("O2"), noinline))
int test_complex_safe(int x, int y, int z) {
    int result = 0;
    
    /* Complex condition that should be safe for if-conversion */
    if ((x * y) > (z + 10)) {
        result = x * z;  /* Doesn't modify x, y, or z */
        global_counter += 4;
    } else {
        result = y * z;
    }
    
    /* Another safe if to create larger basic block */
    if (result < 0) {
        result = -result;
    }
    
    global_result ^= result;
    return result;
}

/* Function 8: Volatile condition variable */
__attribute__((optimize("O3"), noinline))
int test_volatile_condition(volatile int* vptr) {
    int local = *vptr;
    
    if (*vptr > 100) {
        *vptr = 50;  /* MODIFIES volatile memory condition depends on */
        local = 1;
        global_modify += 2;
    } else {
        local = 0;
    }
    
    global_result += local;
    return local;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    /* Use argv to create runtime-dependent values */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Testing if-conversion safety check with seed: %d\n", seed);
    
    /* Initialize test variables with runtime-dependent values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f = (float)(rand() % 100) / 10.0f;
    float g = (float)(rand() % 100) / 10.0f;
    
    int ptr_val = rand() % 200;
    int* dynamic_ptr = malloc(sizeof(int));
    *dynamic_ptr = ptr_val;
    
    volatile int volatile_var = rand() % 150;
    
    /* Call unsafe patterns (should trigger the uncovered safety check) */
    printf("1. Unsafe modification test: ");
    int r1 = test_unsafe_modification(x, y);
    printf("result=%d\n", r1);
    
    printf("2. Pointer condition test: ");
    test_pointer_condition(dynamic_ptr, 100);
    printf("ptr now=%d\n", *dynamic_ptr);
    
    printf("3. Float condition test: ");
    float r3 = test_float_condition(f, g);
    printf("result=%.2f\n", r3);
    
    printf("4. Loop with branch test: ");
    int r4 = test_loop_with_branch(10, seed % 50);
    printf("result=%d\n", r4);
    
    printf("5. Volatile condition test: ");
    int r5 = test_volatile_condition(&volatile_var);
    printf("result=%d\n", r5);
    
    /* Call safe patterns (should allow if-conversion) */
    printf("6. Safe pattern test: ");
    int r6 = test_safe_pattern(x, z);
    printf("result=%d\n", r6);
    
    printf("7. Multiple ifs test: ");
    int r7 = test_multiple_ifs(x, y, z);
    printf("result=%d\n", r7);
    
    printf("8. Complex safe test: ");
    int r8 = test_complex_safe(x, y, z);
    printf("result=%d\n", r8);
    
    /* Final summary */
    printf("\nFinal global_counter: %d\n", global_counter);
    printf("Final global_modify: %d\n", global_modify);
    printf("Final global_result: %d\n", global_result);
    printf("Dynamic pointer value: %d\n", *dynamic_ptr);
    printf("Volatile variable: %d\n", volatile_var);
    
    free(dynamic_ptr);
    
    return 0;
}
