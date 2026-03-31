#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;
int global_cond = 0;

/* Function attributes to control optimization */
__attribute__((noinline, optimize("O2")))
void test_unsafe_int_modification(int x, int y) {
    /* Condition variable x is modified in then-block */
    if (x > 0) {
        x = 10;  /* MODIFIES condition variable */
        global_counter += 1;
    } else {
        global_counter += 2;
    }
    global_result ^= x + y;
}

__attribute__((noinline, optimize("O3")))
void test_unsafe_float_modification(float a, float b) {
    /* Condition variable a is modified in then-block */
    if (a > 0.0f) {
        a = 3.14f;  /* MODIFIES condition variable */
        global_counter += 3;
    } else {
        global_counter += 4;
    }
    global_result ^= (int)(a * b);
}

__attribute__((noinline, optimize("O2")))
void test_safe_pattern(int x, int y) {
    /* Condition variable x is NOT modified in then-block */
    if (x > 0) {
        int temp = y * 2;  /* Does NOT modify x */
        global_counter += temp;
    } else {
        global_counter += y;
    }
    global_result ^= x;
}

__attribute__((noinline, optimize("O3")))
void test_pointer_condition(int *ptr, int threshold) {
    /* Condition uses pointer dereference */
    if (*ptr > threshold) {
        *ptr = threshold;  /* MODIFIES dereferenced condition variable */
        global_counter += 5;
    } else {
        global_counter += 6;
    }
    global_result ^= *ptr;
}

__attribute__((noinline, optimize("O2")))
void test_volatile_condition(volatile int *vptr) {
    /* Condition uses volatile variable */
    if (*vptr > 100) {
        *vptr = 50;  /* MODIFIES volatile condition variable */
        global_counter += 7;
    } else {
        global_counter += 8;
    }
    global_result ^= *vptr;
}

__attribute__((noinline, optimize("O3")))
void test_multi_branch_unsafe(int a, int b, int c) {
    /* Multiple if-statements with unsafe modifications */
    if (a > 0) {
        a = b * c;  /* MODIFIES condition variable */
        global_counter += 9;
    } else {
        global_counter += 10;
    }
    
    if (b < c) {
        b = a + c;  /* MODIFIES condition variable */
        global_counter += 11;
    } else {
        global_counter += 12;
    }
    
    global_result ^= a ^ b ^ c;
}

__attribute__((noinline, optimize("O2")))
void test_builtin_expect_unsafe(int x) {
    /* Using __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        x = x * 2;  /* MODIFIES condition variable */
        global_counter += 13;
    } else {
        global_counter += 14;
    }
    global_result ^= x;
}

__attribute__((noinline, optimize("O3")))
void test_loop_with_unsafe_mod(int iterations) {
    /* Loop to prevent early optimization */
    int i, sum = 0;
    volatile int loop_cond = iterations;
    
    for (i = 0; i < iterations; i++) {
        /* Condition variable modified in then-block inside loop */
        if (loop_cond > i) {
            loop_cond = i;  /* MODIFIES condition variable */
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    global_counter += sum;
    global_result ^= loop_cond;
}

__attribute__((noinline, optimize("O2")))
void test_mixed_safe_unsafe(int x, int y, int z) {
    /* Mix of safe and unsafe patterns */
    if (x > y) {
        /* Safe: doesn't modify x or y */
        int temp = z * 3;
        global_counter += temp;
    } else {
        global_counter += z;
    }
    
    if (y < z) {
        y = x + z;  /* UNSAFE: modifies condition variable y */
        global_counter += 15;
    } else {
        global_counter += 16;
    }
    
    global_result ^= x ^ y ^ z;
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
    
    /* Initialize various condition variables */
    int cond_var1 = rand() % 100;
    float cond_var2 = (rand() % 100) / 10.0f;
    volatile int cond_var3 = rand() % 100;
    int array[5] = {rand() % 100, rand() % 100, rand() % 100, 
                    rand() % 100, rand() % 100};
    int *ptr = &array[0];
    volatile int volatile_var = rand() % 200;
    
    printf("Starting tests with seed: %d\n", seed);
    printf("Initial values: cond_var1=%d, cond_var2=%.2f, cond_var3=%d\n",
           cond_var1, cond_var2, cond_var3);
    
    /* Call test functions with runtime-determined values */
    test_unsafe_int_modification(cond_var1, rand() % 50);
    test_unsafe_float_modification(cond_var2, (rand() % 50) / 10.0f);
    test_safe_pattern(cond_var1, rand() % 50);
    test_pointer_condition(ptr, 50);
    test_volatile_condition(&volatile_var);
    test_multi_branch_unsafe(cond_var1, rand() % 50, rand() % 50);
    test_builtin_expect_unsafe(rand() % 100);
    test_loop_with_unsafe_mod(10 + (rand() % 20));
    test_mixed_safe_unsafe(cond_var1, rand() % 50, rand() % 50);
    
    /* Additional calls with different patterns */
    for (int i = 0; i < 5; i++) {
        int temp = rand() % 100;
        if (temp > 50) {
            test_safe_pattern(temp, i);
        } else {
            test_unsafe_int_modification(temp, i);
        }
    }
    
    printf("Final counter: %d\n", global_counter);
    printf("Final result: %d\n", global_result);
    
    return global_result;
}
