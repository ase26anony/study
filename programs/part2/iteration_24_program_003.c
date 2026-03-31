#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Global variables to prevent optimization
volatile int global_accumulator = 0;
volatile int global_cond = 0;

// Function attributes to control optimization
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification_int(int x, int y) {
    int result = 0;
    
    // This should trigger the safety check - x is modified in then block
    if (x > 0) {
        x = y * 2;  // Modifies condition variable
        result = 100;
    } else {
        result = 200;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O3")))
float test_unsafe_modification_float(float a, float b) {
    float result = 0.0f;
    
    // Float condition with modification in then block
    if (a > 1.0f) {
        a = b * 3.14f;  // Modifies condition variable
        result = a + b;
    } else {
        result = a - b;
    }
    
    global_accumulator += (int)result;
    return result;
}

__attribute__((noinline, optimize("O2")))
int test_pointer_condition(int *ptr, int threshold) {
    int local = *ptr;
    int result = 0;
    
    // Pointer dereference in condition, modified in then block
    if (local > threshold) {
        *ptr = threshold;  // Modifies the memory location used in condition
        result = 1;
    } else {
        result = 0;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O3")))
int test_volatile_condition(volatile int *vptr) {
    int result = 0;
    volatile int cond = *vptr;
    
    // Volatile variable in condition
    if (cond > 50) {
        *vptr = 25;  // Modifies volatile condition variable
        result = 999;
    } else {
        result = 111;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O2")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    
    // Safe pattern - condition variable not modified in then block
    if (x > y) {
        result = x * 2;  // Doesn't modify x or y
    } else {
        result = y * 3;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O3")))
int test_mixed_conditions(int a, int b, int c) {
    int result = 0;
    
    // Multiple if-statements in sequence
    if (a > 10) {
        a = b + c;  // Modifies condition variable
        result += 10;
    } else {
        result += 20;
    }
    
    if (b < 20) {
        // b not modified here - safe
        result += b * 2;
    } else {
        result += c * 3;
    }
    
    if (c != 0) {
        c = a + b;  // Modifies condition variable
        result += 100;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O2")))
int test_with_builtin_expect(int x, int y) {
    int result = 0;
    
    // Using __builtin_expect to influence branch prediction
    if (__builtin_expect(x > 0, 1)) {
        x = y * x;  // Modifies condition variable
        result = 50;
    } else {
        result = 25;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O3")))
int test_increment_modification(int x, int y) {
    int result = 0;
    
    // Increment modifies condition variable
    if (x++ > y) {  // x is modified in the evaluation
        x += 5;     // Further modification in then block
        result = 66;
    } else {
        result = 77;
    }
    
    global_accumulator += result;
    return result;
}

__attribute__((noinline, optimize("O2")))
int test_loop_with_unsafe_if(int iterations) {
    int sum = 0;
    int cond_var = 10;
    
    // Loop to prevent early optimization
    for (int i = 0; i < iterations; i++) {
        // This if inside loop should reach if-conversion pass
        if (cond_var > i) {
            cond_var = i * 2;  // Modifies condition variable
            sum += i;
        } else {
            sum += cond_var;
        }
        
        // Additional code to make block non-trivial
        sum = (sum * 3) / 2;
    }
    
    global_accumulator += sum;
    return sum;
}

__attribute__((noinline, optimize("O3")))
int test_nested_conditions(int a, int b, int c) {
    int result = 0;
    
    if (a > b) {
        if (b > c) {
            a = c;  // Modifies outer condition variable
            result = 1;
        } else {
            result = 2;
        }
    } else {
        result = 3;
    }
    
    global_accumulator += result;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    // Use argv or time for runtime variability
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    // Initialize variables with runtime-dependent values
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    volatile int volatile_var = rand() % 100;
    int array[3] = {rand() % 100, rand() % 100, rand() % 100};
    int *ptr = array;
    
    printf("Starting tests with seed: %d\n", seed);
    printf("Initial values: x=%d, y=%d, z=%d, f1=%.2f, f2=%.2f\n", x, y, z, f1, f2);
    
    // Call test functions - mix of safe and unsafe patterns
    int r1 = test_unsafe_modification_int(x, y);
    float r2 = test_unsafe_modification_float(f1, f2);
    int r3 = test_pointer_condition(&x, 50);
    int r4 = test_volatile_condition(&volatile_var);
    int r5 = test_safe_pattern(y, z);
    int r6 = test_mixed_conditions(x, y, z);
    int r7 = test_with_builtin_expect(z, x);
    int r8 = test_increment_modification(y, z);
    int r9 = test_loop_with_unsafe_if(10 + (rand() % 5));
    int r10 = test_nested_conditions(x, y, z);
    
    printf("Results: %d, %.2f, %d, %d, %d, %d, %d, %d, %d, %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return global_accumulator > 0 ? 0 : 1;
}
