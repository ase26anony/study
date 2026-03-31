#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Test 1: Unsafe modification - modifies condition variable in then block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification(int x, int y) {
    int result = 0;
    
    /* Loop to prevent early optimization */
    for (int i = 0; i < 3; i++) {
        /* Condition variable x is modified in the then block */
        if (x > y) {
            x = x + 1;  /* MODIFIES condition variable! */
            result += 10;
            global_counter++;
        } else {
            result += 5;
            global_counter--;
        }
        
        /* Mix with other operations to encourage if-conversion */
        y = y * 2 - 1;
    }
    
    return result;
}

/* Test 2: Safe pattern - condition variable not modified in then block */
__attribute__((noinline, optimize("O3")))
int test_safe_pattern(int a, int b) {
    int local = a;
    int result = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(local > b, 1)) {
        /* Does NOT modify local (condition variable) */
        result = a * 2;
        global_result += result;
    } else {
        result = b * 3;
        global_result -= result;
    }
    
    return result;
}

/* Test 3: Pointer-based condition with unsafe modification */
__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int sum = 0;
    
    /* Multiple if-statements in sequence */
    for (int i = 0; i < 4; i++) {
        /* Condition depends on pointer dereference */
        if (*ptr > threshold) {
            *ptr = *ptr - 1;  /* MODIFIES dereferenced condition! */
            sum += 100;
        } else {
            sum += 50;
            threshold++;
        }
        
        /* Another if-statement to create larger basic block */
        if (sum < 200) {
            sum += 10;
        }
    }
    
    return sum;
}

/* Test 4: Float condition with modification */
__attribute__((noinline, optimize("O3")))
float test_float_condition(float f1, float f2) {
    float result = 0.0f;
    
    /* Use volatile to prevent reordering */
    volatile float cond = f1;
    
    if (cond > f2) {
        cond = cond * 1.5f;  /* MODIFIES condition variable! */
        result = f1 + f2;
        global_counter += 2;
    } else {
        result = f1 - f2;
        global_counter -= 2;
    }
    
    return result;
}

/* Test 5: Complex expression in condition */
__attribute__((noinline, optimize("O2")))
int test_complex_condition(int x, int y, int z) {
    int result = 0;
    
    /* Complex condition expression */
    if ((x * y) > (z + 10)) {
        x = x / 2;  /* Modifies x which is part of condition expression */
        result = x + y + z;
    } else {
        result = x - y - z;
        y++;  /* Modifies y, but not in the then block */
    }
    
    return result;
}

/* Test 6: Nested if with modification */
__attribute__((noinline, optimize("O3")))
int test_nested_modification(int a, int b, int c) {
    int val = a;
    int total = 0;
    
    /* Outer if */
    if (val > b) {
        /* Inner if */
        if (val < c) {
            val = val * 2;  /* MODIFIES condition variable! */
            total += 100;
        } else {
            total += 50;
        }
        /* This should still trigger the check in outer block's then section */
    } else {
        total += 25;
    }
    
    return total;
}

/* Test 7: Safe pattern with multiple branches */
__attribute__((noinline, optimize("O2")))
int test_safe_multibranch(int x) {
    int result = 0;
    
    /* Multiple independent if-statements */
    if (x > 0) {
        result += 10;  /* Safe - doesn't modify x */
    }
    
    if (x < 100) {
        result += 20;  /* Safe - doesn't modify x */
    }
    
    if (x == 50) {
        result += 30;  /* Safe - doesn't modify x */
    }
    
    return result;
}

/* Test 8: Volatile condition variable */
__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int* v) {
    int score = 0;
    
    /* Loop with volatile access */
    for (int i = 0; i < 3; i++) {
        if (*v > 0) {
            *v = *v - 1;  /* MODIFIES volatile condition! */
            score += 100;
        } else {
            score += 10;
        }
    }
    
    return score;
}

int main(int argc, char *argv[]) {
    int seed;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Initialize test variables with random values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    int ptr_val = rand() % 100;
    volatile int volatile_val = rand() % 50;
    
    int total = 0;
    
    printf("Starting if-conversion tests with seed %d\n", seed);
    printf("Initial values: x=%d, y=%d, z=%d, f1=%.2f, f2=%.2f\n", 
           x, y, z, f1, f2);
    
    /* Run all test functions */
    total += test_unsafe_modification(x, y);
    printf("After unsafe_modification: total=%d, global_counter=%d\n", 
           total, global_counter);
    
    total += test_safe_pattern(x, z);
    printf("After safe_pattern: total=%d, global_result=%d\n", 
           total, global_result);
    
    total += test_pointer_condition(&ptr_val, 30);
    printf("After pointer_condition: total=%d, ptr_val=%d\n", 
           total, ptr_val);
    
    total += (int)test_float_condition(f1, f2);
    printf("After float_condition: total=%d, global_counter=%d\n", 
           total, global_counter);
    
    total += test_complex_condition(x, y, z);
    printf("After complex_condition: total=%d\n", total);
    
    total += test_nested_modification(x, y, z);
    printf("After nested_modification: total=%d\n", total);
    
    total += test_safe_multibranch(x);
    printf("After safe_multibranch: total=%d\n", total);
    
    total += test_volatile_condition(&volatile_val);
    printf("After volatile_condition: total=%d, volatile_val=%d\n", 
           total, volatile_val);
    
    printf("Final total: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
