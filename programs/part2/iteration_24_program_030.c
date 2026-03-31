#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables for condition testing */
volatile int global_cond = 0;
int global_result = 0;

/* Function with unsafe modification in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_modification_int(int x, int y) {
    int result = 0;
    
    /* Condition variable x is modified in then-block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable */
        result = y * 2;
    } else {
        result = y + 5;
    }
    
    /* Use result to prevent dead code elimination */
    global_result += result;
    return result;
}

/* Function with pointer-based condition modification */
__attribute__((noinline, optimize("O3")))
int test_unsafe_pointer_modification(int* ptr, int threshold) {
    int local = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold - 1;  /* MODIFIES condition expression */
        local = 100;
    } else {
        local = 200;
    }
    
    global_result += local;
    return local;
}

/* Function with float condition modification */
__attribute__((noinline, optimize("O2")))
float test_unsafe_float_modification(float f, float inc) {
    float result = 0.0f;
    
    /* Float condition variable modified in then-block */
    if (f > 0.5f) {
        f += inc;  /* MODIFIES condition variable */
        result = f * 2.0f;
    } else {
        result = f / 2.0f;
    }
    
    global_result += (int)result;
    return result;
}

/* Function with volatile condition modification */
__attribute__((noinline, optimize("O3")))
int test_volatile_modification(int seed) {
    volatile int vol_cond = seed;
    int output = 0;
    
    /* Volatile condition variable */
    if (vol_cond % 2 == 0) {
        vol_cond++;  /* MODIFIES condition variable */
        output = seed * 3;
    } else {
        output = seed * 7;
    }
    
    global_result += output;
    return output;
}

/* Function with multiple related if-statements */
__attribute__((noinline, optimize("O2")))
int test_multiple_ifs_with_modification(int a, int b, int c) {
    int x = a;
    int result = 0;
    
    /* First if: safe (x not modified in then-block) */
    if (x > 10) {
        result += b;
    } else {
        result += c;
    }
    
    /* Second if: unsafe (x modified in then-block) */
    if (x < 20) {
        x = 25;  /* MODIFIES condition variable for next if */
        result += 5;
    } else {
        result += 10;
    }
    
    /* Third if: uses modified x */
    if (x == 25) {
        result += 100;
    }
    
    global_result += result;
    return result;
}

/* Function with builtin expect and modification */
__attribute__((noinline, optimize("O3")))
int test_builtin_expect_with_modification(int val) {
    int temp = val;
    
    /* Hint at branch prediction */
    if (__builtin_expect(temp > 50, 1)) {
        temp = 0;  /* MODIFIES condition variable */
        return 1;
    } else {
        return 0;
    }
}

/* SAFE pattern: condition variable NOT modified in then-block */
__attribute__((noinline, optimize("O2")))
int test_safe_pattern(int x, int y) {
    int result = 0;
    int temp = x;  /* Copy for use in condition */
    
    if (temp > 0) {
        /* Does NOT modify temp/x */
        result = y * 3;
    } else {
        result = y - 3;
    }
    
    global_result += result;
    return result;
}

/* Function with loop to prevent early optimization */
__attribute__((noinline, optimize("O2")))
int test_in_loop_with_modification(int iterations, int start) {
    int counter = start;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition variable counter modified in then-block */
        if (counter % 3 == 0) {
            counter += i;  /* MODIFIES condition variable */
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Additional operation to create larger basic block */
        counter = (counter * 13 + 7) % 100;
    }
    
    global_result += sum;
    return sum;
}

/* Function with complex condition expression */
__attribute__((noinline, optimize("O3")))
int test_complex_condition_modification(int a, int b, int c) {
    int x = a;
    int y = b;
    int result = 0;
    
    /* Complex condition with multiple variables */
    if ((x > y) && (x < c)) {
        x = y + c;  /* MODIFIES part of condition expression */
        result = x * 2;
    } else {
        result = y + c;
    }
    
    global_result += result;
    return result;
}

/* Main function with runtime-determined paths */
int main(int argc, char *argv[]) {
    int seed;
    
    /* Use argv or time for runtime variation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    float cond_var2 = (rand() % 100) / 100.0f;
    volatile int cond_var3 = rand() % 50;
    int array[5] = {10, 20, 30, 40, 50};
    int *ptr = &array[2];
    
    printf("Starting tests with seed: %d\n", seed);
    
    /* Call unsafe modification functions */
    int r1 = test_unsafe_modification_int(cond_var1, 5);
    printf("test_unsafe_modification_int: %d\n", r1);
    
    int r2 = test_unsafe_pointer_modification(ptr, 25);
    printf("test_unsafe_pointer_modification: %d\n", r2);
    
    float r3 = test_unsafe_float_modification(cond_var2, 0.1f);
    printf("test_unsafe_float_modification: %.2f\n", r3);
    
    int r4 = test_volatile_modification(cond_var3);
    printf("test_volatile_modification: %d\n", r4);
    
    int r5 = test_multiple_ifs_with_modification(15, 20, 25);
    printf("test_multiple_ifs_with_modification: %d\n", r5);
    
    int r6 = test_builtin_expect_with_modification(75);
    printf("test_builtin_expect_with_modification: %d\n", r6);
    
    int r7 = test_in_loop_with_modification(10, seed % 20);
    printf("test_in_loop_with_modification: %d\n", r7);
    
    int r8 = test_complex_condition_modification(10, 20, 30);
    printf("test_complex_condition_modification: %d\n", r8);
    
    /* Call safe pattern for comparison */
    int r9 = test_safe_pattern(cond_var1, 10);
    printf("test_safe_pattern: %d\n", r9);
    
    printf("Global result accumulator: %d\n", global_result);
    
    return global_result > 0 ? 0 : 1;
}
