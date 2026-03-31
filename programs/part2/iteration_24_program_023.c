#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_cond = 0;

/* Function prototypes */
void __attribute__((noinline)) test_unsafe_int_modification(int seed);
void __attribute__((noinline)) test_safe_pattern(int seed);
void __attribute__((noinline)) test_pointer_condition(int seed);
void __attribute__((noinline)) test_volatile_condition(int seed);
void __attribute__((noinline)) test_float_condition(int seed);
void __attribute__((noinline)) test_multi_branch(int seed);
void __attribute__((noinline)) test_nested_unsafe(int seed);
void __attribute__((optimize("O3"))) test_aggressive_opt(int seed);
void __attribute__((optimize("O2"))) test_O2_optimized(int seed);

/* Test 1: Unsafe modification of condition variable in then-block */
void __attribute__((noinline)) test_unsafe_int_modification(int seed) {
    int x = seed % 100;
    int y = 0;
    
    /* Use runtime condition to prevent constant folding */
    if (seed > 50) {
        x = 100;  /* MODIFIES condition variable x */
        y = x * 2;
    } else {
        y = x / 2;
    }
    
    /* Mix with other operations to encourage if-conversion */
    int result = y + (x & 0xF);
    global_accumulator += result;
    
    /* Add unpredictable branch to prevent optimization */
    if (__builtin_expect((seed & 1), 0)) {
        global_accumulator ^= result;
    }
}

/* Test 2: Safe pattern - condition variable not modified in then-block */
void __attribute__((noinline)) test_safe_pattern(int seed) {
    int a = seed;
    int b = 0;
    int c = a % 50;
    
    /* Condition variable c is NOT modified in then-block */
    if (c > 25) {
        b = a * 3;  /* Does NOT modify c */
    } else {
        b = a * 2;
        c = b;  /* Modifies c in else-block (should be okay) */
    }
    
    global_accumulator += b + c;
}

/* Test 3: Pointer-based condition with unsafe modification */
void __attribute__((noinline)) test_pointer_condition(int seed) {
    int data[4] = {seed, seed + 1, seed + 2, seed + 3};
    int *ptr = &data[0];
    int result = 0;
    
    /* Condition depends on pointer dereference */
    if (*ptr > 50) {
        *ptr = 0;  /* MODIFIES the dereferenced value used in condition */
        result = 100;
    } else {
        result = 200;
    }
    
    global_accumulator += result + data[0];
}

/* Test 4: Volatile variable as condition */
void __attribute__((noinline)) test_volatile_condition(int seed) {
    volatile int v = seed;
    int temp = 0;
    
    /* Volatile read in condition */
    if (v > 100) {
        v = 50;  /* MODIFIES volatile condition variable */
        temp = v * 2;
    } else {
        temp = v * 3;
    }
    
    global_accumulator += temp;
}

/* Test 5: Float condition with modification */
void __attribute__((noinline)) test_float_condition(int seed) {
    float f = (float)seed / 10.0f;
    int result = 0;
    
    /* Float comparison in condition */
    if (f > 5.0f) {
        f = 0.0f;  /* MODIFIES float condition variable */
        result = 100;
    } else {
        result = 200;
    }
    
    global_accumulator += result + (int)f;
}

/* Test 6: Multiple related if-statements in sequence */
void __attribute__((noinline)) test_multi_branch(int seed) {
    int x = seed;
    int y = 0;
    
    /* First if: unsafe modification */
    if (x % 3 == 0) {
        x = x * 2;  /* Modifies x used in next condition */
        y += 10;
    } else {
        y += 5;
    }
    
    /* Second if: uses modified x */
    if (x > 100) {
        y += 20;
    } else {
        y += 30;
    }
    
    /* Third if: safe pattern */
    int z = seed % 20;
    if (z > 10) {
        y += 40;
    } else {
        y += 50;
    }
    
    global_accumulator += y;
}

/* Test 7: Nested if with unsafe modification in inner block */
void __attribute__((noinline)) test_nested_unsafe(int seed) {
    int a = seed;
    int b = 0;
    
    if (a > 0) {
        if (a < 100) {
            a = 50;  /* Modifies outer condition variable */
            b = a * 2;
        } else {
            b = a * 3;
        }
    } else {
        b = -a;
    }
    
    global_accumulator += b;
}

/* Test 8: Aggressive optimization with O3 */
void __attribute__((optimize("O3"))) test_aggressive_opt(int seed) {
    int x = seed;
    int y = 0;
    
    /* Loop to create larger basic block */
    for (int i = 0; i < 10; i++) {
        if (x > i * 10) {
            x = i;  /* Modifies condition variable inside loop */
            y += x;
        } else {
            y -= i;
        }
    }
    
    global_accumulator += y;
}

/* Test 9: O2 optimized with builtin expect */
void __attribute__((optimize("O2"))) test_O2_optimized(int seed) {
    int counter = seed;
    int sum = 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(counter > 75, 1)) {
        counter = 0;  /* Modifies condition variable */
        sum = 100;
    } else {
        sum = 200;
    }
    
    /* Additional arithmetic to create conditional move candidates */
    int temp = (counter > 50) ? sum * 2 : sum / 2;
    global_accumulator += temp;
}

int main(int argc, char *argv[]) {
    /* Use argv for runtime variability to prevent constant folding */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Call test functions multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        int local_seed = rand() % 100;
        
        test_unsafe_int_modification(local_seed);
        test_safe_pattern(local_seed);
        test_pointer_condition(local_seed);
        test_volatile_condition(local_seed);
        test_float_condition(local_seed);
        test_multi_branch(local_seed);
        test_nested_unsafe(local_seed);
        test_aggressive_opt(local_seed);
        test_O2_optimized(local_seed);
    }
    
    printf("Final accumulator value: %d\n", global_accumulator);
    printf("Seed used: %d\n", seed);
    
    return global_accumulator != 0 ? 0 : 1;
}
