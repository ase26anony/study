#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for condition testing */
volatile int global_cond = 0;
int global_result = 0;

/* Function prototypes */
int __attribute__((noinline, optimize("O2"))) 
test_unsafe_int_modification(int x, int y);
int __attribute__((noinline, optimize("O3"))) 
test_safe_pattern(int x, int y);
int __attribute__((noinline, optimize("O2"))) 
test_volatile_condition(void);
int __attribute__((noinline, optimize("O3"))) 
test_pointer_dereference(int *ptr, int threshold);
int __attribute__((noinline, optimize("O2"))) 
test_mixed_conditions(int a, int b, int c);
int __attribute__((noinline, optimize("O3"))) 
test_float_condition(float f1, float f2);
int __attribute__((noinline, optimize("O2"))) 
test_nested_unsafe(int x, int y, int z);
int __attribute__((noinline, optimize("O3"))) 
test_loop_with_unsafe_mod(int iterations, int seed);

/* Test 1: Unsafe modification - modifies condition variable in then-block */
int __attribute__((noinline, optimize("O2"))) 
test_unsafe_int_modification(int x, int y) {
    int result = 0;
    
    /* This should trigger the safety check: x is modified in then-block */
    if (x > 0) {
        x = y * 2;  /* MODIFIES condition variable x */
        result = x + 10;
    } else {
        result = y - 5;
    }
    
    /* Add some computation to prevent dead code elimination */
    result += (x & 1);
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
int __attribute__((noinline, optimize("O3"))) 
test_safe_pattern(int x, int y) {
    int result = 0;
    int temp = x;  /* Copy to avoid modifying original */
    
    /* This should pass the safety check: temp is not modified in then-block */
    if (temp > 0) {
        result = y * 3;  /* Does NOT modify temp */
        result += 7;
    } else {
        result = y / 2;
    }
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(result > 100, 0)) {
        result -= 50;
    }
    
    return result + temp;
}

/* Test 3: Volatile condition variable */
int __attribute__((noinline, optimize("O2"))) 
test_volatile_condition(void) {
    volatile int local_volatile = global_cond;
    int result = 0;
    
    /* Volatile read in condition */
    if (local_volatile > 10) {
        local_volatile = 5;  /* Modifies volatile condition variable */
        result = 100;
    } else {
        result = 200;
    }
    
    /* Force another read to prevent optimization */
    global_cond = local_volatile;
    return result;
}

/* Test 4: Pointer dereference in condition */
int __attribute__((noinline, optimize("O3"))) 
test_pointer_dereference(int *ptr, int threshold) {
    int result = 0;
    
    /* Pointer dereference in condition */
    if (*ptr > threshold) {
        *ptr = threshold;  /* Modifies memory pointed by condition expression */
        result = 1;
    } else {
        result = 0;
    }
    
    /* Additional computation to encourage if-conversion */
    result = result * 2 + (*ptr & 1);
    return result;
}

/* Test 5: Mixed conditions with one unsafe modification */
int __attribute__((noinline, optimize("O2"))) 
test_mixed_conditions(int a, int b, int c) {
    int result = 0;
    
    /* Sequence of if-statements */
    if (a > b) {
        a = b + c;  /* Modifies a used in next condition */
        result += 10;
    } else {
        result -= 5;
    }
    
    if (b < c) {
        /* b not modified here - safe */
        result += 20;
    } else {
        result += 30;
    }
    
    if (c != 0) {
        c = a + b;  /* Modifies c */
        result *= 2;
    }
    
    return result + a + b + c;
}

/* Test 6: Float condition with modification */
int __attribute__((noinline, optimize("O3"))) 
test_float_condition(float f1, float f2) {
    float result_f = 0.0f;
    
    /* Float comparison in condition */
    if (f1 > f2) {
        f1 = f2 * 1.5f;  /* Modifies float condition variable */
        result_f = f1 + 10.0f;
    } else {
        result_f = f2 - 5.0f;
    }
    
    /* Convert to int for return */
    return (int)result_f + (int)f1;
}

/* Test 7: Nested if with unsafe modification in inner block */
int __attribute__((noinline, optimize("O2"))) 
test_nested_unsafe(int x, int y, int z) {
    int result = 0;
    
    if (x > y) {
        if (y > z) {
            x = z * 2;  /* Modifies outer condition variable */
            result = 100;
        } else {
            result = 200;
        }
        result += x;  /* Uses modified x */
    } else {
        result = 300;
    }
    
    return result;
}

/* Test 8: Loop with unsafe modification inside */
int __attribute__((noinline, optimize("O3"))) 
test_loop_with_unsafe_mod(int iterations, int seed) {
    int sum = 0;
    int counter = seed;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < iterations; i++) {
        /* Condition variable counter modified in then-block */
        if (counter % 3 == 0) {
            counter += i;  /* Modifies condition variable */
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Additional branch to create complex control flow */
        if (__builtin_expect((sum & 1) == 0, 1)) {
            counter ^= 1;  /* Flip LSB */
        }
    }
    
    return sum + counter;
}

/* Main function with runtime-dependent execution */
int main(int argc, char *argv[]) {
    int seed = 0;
    int total_result = 0;
    
    /* Use argv for runtime variability to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]) % 100;
    } else {
        seed = rand() % 100;
    }
    
    /* Initialize condition variables */
    int cond_var1 = seed + 10;
    int cond_var2 = seed - 5;
    float float_var1 = (float)seed * 1.5f;
    float float_var2 = (float)seed * 0.8f;
    int array[3] = {seed, seed + 1, seed + 2};
    
    /* Call test functions with runtime-dependent values */
    total_result += test_unsafe_int_modification(cond_var1, cond_var2);
    total_result += test_safe_pattern(cond_var1 + 1, cond_var2 - 1);
    
    global_cond = seed * 2;
    total_result += test_volatile_condition();
    
    total_result += test_pointer_dereference(&array[0], seed + 20);
    total_result += test_mixed_conditions(seed, seed + 3, seed + 7);
    total_result += test_float_condition(float_var1, float_var2);
    total_result += test_nested_unsafe(seed, seed + 2, seed + 4);
    total_result += test_loop_with_unsafe_mod(10 + (seed % 5), seed);
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %d (seed: %d)\n", total_result, seed);
    
    /* Store in global to ensure all code has side effects */
    global_result = total_result;
    
    return total_result > 0 ? 0 : 1;
}
