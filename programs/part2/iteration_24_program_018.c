#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_accumulator = 0;
volatile int global_cond = 0;

/* Test 1: Unsafe modification - modifies condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in then-block */
        if (x > 0) {
            x = x + y;  /* MODIFIES condition variable */
            result += 10;
            global_counter++;
        } else {
            result += 5;
        }
        /* Mix with other operations */
        y = y * 2 + 1;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int local = a;
    int output = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local > 10, 1)) {
        output = b * 2;  /* Does NOT modify local */
        global_counter += 2;
    } else {
        local = b;  /* Modifies in else-block, not then-block */
        output = b / 2;
    }
    
    /* Multiple related if-statements */
    if (output > 100) {
        output = 100;
    }
    
    global_accumulator += output;
    return output;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int val = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold;  /* MODIFIES the dereferenced value */
        val = *ptr * 3;
        global_counter += 3;
    } else {
        val = *ptr * 2;
    }
    
    global_accumulator += val;
    return val;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f, float inc) {
    float result = 0.0f;
    
    /* Use volatile to prevent optimization */
    volatile float cond_var = f;
    
    if (cond_var > 0.5f) {
        cond_var += inc;  /* MODIFIES condition variable */
        result = cond_var * 2.0f;
        global_counter++;
    } else {
        result = cond_var * 0.5f;
    }
    
    global_accumulator += (int)result;
    return result;
}

/* Test 5: Complex expression in condition */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int a, int b, int c) {
    int x = a;
    int y = b;
    int z = 0;
    
    /* Complex condition expression */
    if ((x * y) > (a + b + c)) {
        x = x / 2;  /* Modifies x which is part of condition expression */
        z = x + y;
        global_counter += 4;
    } else {
        y = y * 2;
        z = x - y;
    }
    
    /* Another if-statement to create larger basic block */
    if (z > 1000) {
        z = 1000;
    }
    
    global_accumulator += z;
    return z;
}

/* Test 6: Safe with multiple condition variables */
__attribute__((noinline, optimize("O3")))
int test_safe_multi_condition(int p, int q, int r) {
    int result = 0;
    
    /* Multiple variables in condition, none modified in then-block */
    if (p > q && q < r) {
        result = p + r;  /* Doesn't modify p, q, or r */
        global_counter += 5;
    } else {
        result = q - p;
        /* Modify in else-block is safe */
        p = r;
    }
    
    global_accumulator += result;
    return result;
}

/* Test 7: Volatile condition variable */
__attribute__((noinline, optimize("O2")))
int test_volatile_condition(volatile int *vptr) {
    int local = 0;
    
    /* Condition uses volatile access */
    if (*vptr > 50) {
        *vptr = 25;  /* MODIFIES volatile condition variable */
        local = 100;
        global_counter += 6;
    } else {
        local = 50;
    }
    
    global_accumulator += local;
    return local;
}

/* Test 8: Nested if with modification in inner block */
__attribute__((noinline, optimize("O3")))
int test_nested_modification(int base, int mod) {
    int temp = base;
    int out = 0;
    
    if (temp > 0) {
        if (mod > 0) {
            temp = mod;  /* MODIFIES outer condition variable */
            out = temp * 2;
            global_counter += 7;
        } else {
            out = temp;
        }
    } else {
        out = -temp;
    }
    
    global_accumulator += out;
    return out;
}

int main(int argc, char *argv[]) {
    int seed;
    
    /* Use argv or time for runtime-determined values */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    int cond_var2 = rand() % 100;
    float cond_var3 = (rand() % 100) / 100.0f;
    volatile int cond_var4 = rand() % 100;
    int array[3] = {rand() % 100, rand() % 100, rand() % 100};
    int *ptr = array;
    
    printf("Starting tests with seed: %d\n", seed);
    printf("Initial values: %d, %d, %.2f, %d\n", 
           cond_var1, cond_var2, cond_var3, cond_var4);
    
    /* Call test functions with runtime values */
    int r1 = test_unsafe_modification(cond_var1, cond_var2);
    int r2 = test_safe_pattern(cond_var1, cond_var2);
    int r3 = test_pointer_condition(&array[0], 50);
    float r4 = test_float_condition(cond_var3, 0.1f);
    int r5 = test_complex_condition(cond_var1, cond_var2, array[2]);
    int r6 = test_safe_multi_condition(cond_var1, cond_var2, array[1]);
    int r7 = test_volatile_condition(&cond_var4);
    int r8 = test_nested_modification(cond_var1, cond_var2);
    
    /* Force all results to be used */
    int total = r1 + r2 + r3 + (int)r4 + r5 + r6 + r7 + r8;
    
    printf("Results: %d, %d, %d, %.2f, %d, %d, %d, %d\n",
           r1, r2, r3, r4, r5, r6, r7, r8);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Total: %d\n", total);
    
    /* Return value based on results to prevent dead code elimination */
    return (total > 1000) ? 0 : 1;
}
