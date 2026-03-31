/* test_ifcvt_safety.c
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -da -o test_ifcvt test_ifcvt_safety.c
 * Also try: gcc -O3 -march=core2 -fno-tree-loop-if-convert -fno-tree-loop-vectorize -o test_ifcvt test_ifcvt_safety.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* ========== UNSAFE PATTERNS (should trigger the uncovered check) ========== */

/* Pattern 1: Direct modification of condition variable in then-block */
__attribute__((noinline, optimize("O2")))
int test_unsafe_direct_mod(int x, int y) {
    int result = 0;
    /* Condition variable x is modified in then-block */
    if (x > 0) {
        x = y * 2;  /* MODIFIES condition variable */
        result = x + 10;
        global_acc += result;
    } else {
        result = y - 5;
        global_acc += result;
    }
    return result;
}

/* Pattern 2: Increment of condition variable */
__attribute__((noinline, optimize("O3")))
float test_unsafe_increment(float a, float b) {
    float res = 0.0f;
    /* Using __builtin_expect to influence branch prediction */
    if (__builtin_expect(a != 0.0f, 1)) {
        a += 1.5f;  /* MODIFIES condition variable */
        res = a * b;
        global_acc += (int)res;
    } else {
        res = b / 2.0f;
        global_acc += (int)res;
    }
    return res;
}

/* Pattern 3: Pointer dereference that modifies condition memory */
__attribute__((noinline, optimize("O2")))
int test_unsafe_pointer(int *ptr, int threshold) {
    int val = *ptr;
    int output = 0;
    
    if (val < threshold) {
        *ptr = threshold + 1;  /* MODIFIES memory pointed by condition variable */
        output = val * 2;
        global_acc += output;
    } else {
        output = val / 2;
        global_acc += output;
    }
    return output;
}

/* Pattern 4: Volatile condition variable modification */
__attribute__((noinline, optimize("O3")))
int test_unsafe_volatile(volatile int *v) {
    int check = *v;
    int ret = 0;
    
    if (check > 100) {
        *v = 50;  /* MODIFIES volatile condition variable */
        ret = check - 50;
        global_acc += ret;
    } else {
        ret = check + 100;
        global_acc += ret;
    }
    return ret;
}

/* Pattern 5: Multiple related if-statements with modification */
__attribute__((noinline, optimize("O2")))
int test_unsafe_multiple_ifs(int x, int y, int z) {
    int total = 0;
    
    /* First if: modifies condition variable */
    if (x > y) {
        x = z;  /* MODIFIES condition variable for next if */
        total += 10;
    } else {
        total -= 5;
    }
    
    /* Second if: uses modified variable */
    if (x < z) {
        total += 20;
    }
    
    global_acc += total;
    return total;
}

/* ========== SAFE PATTERNS (should pass the safety check) ========== */

/* Pattern 6: Safe pattern - condition variable not modified */
__attribute__((noinline, optimize("O2")))
int test_safe_pattern(int a, int b) {
    int result = 0;
    /* Condition variable a is NOT modified in then-block */
    if (a > b) {
        result = a * 2;  /* Uses but doesn't modify a */
        global_acc += result;
    } else {
        result = b * 3;
        global_acc += result;
    }
    return result;
}

/* Pattern 7: Safe pattern with different variable in then-block */
__attribute__((noinline, optimize("O3")))
float test_safe_different_var(float x, float y) {
    float res = 0.0f;
    float temp = 0.0f;
    
    if (x > 0.0f) {
        temp = y * 2.0f;  /* Modifies different variable */
        res = x + temp;
        global_acc += (int)res;
    } else {
        res = y - x;
        global_acc += (int)res;
    }
    return res;
}

/* Pattern 8: Safe pattern in loop context */
__attribute__((noinline, optimize("O2")))
int test_safe_in_loop(int n, int *data) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int val = data[i];
        /* Condition variable val is not modified in then-block */
        if (val % 2 == 0) {
            sum += val * 2;
        } else {
            sum += val;
        }
    }
    
    global_acc += sum;
    return sum;
}

/* Pattern 9: Complex safe pattern with function call */
__attribute__((noinline, optimize("O3")))
int test_safe_with_external(int x, int y) {
    extern int external_helper(int);
    int result = 0;
    
    if (x > y) {
        result = external_helper(x);  /* Function doesn't modify x */
        global_acc += result;
    } else {
        result = external_helper(y);
        global_acc += result;
    }
    return result;
}

/* ========== MIXED PATTERNS ========== */

/* Pattern 10: Mixed safe/unsafe in same function */
__attribute__((noinline, optimize("O2")))
int test_mixed_patterns(int a, int b, int c) {
    int score = 0;
    
    /* UNSAFE: modifies condition variable */
    if (a > 10) {
        a = b;  /* MODIFIES condition variable */
        score += 5;
    }
    
    /* SAFE: doesn't modify condition variable */
    if (b < c) {
        score += 10;
    }
    
    /* UNSAFE: modifies through pointer */
    int *ptr = &a;
    if (*ptr > 0) {
        *ptr = 0;  /* MODIFIES dereferenced condition */
        score += 15;
    }
    
    global_acc += score;
    return score;
}

/* Pattern 11: Nested if with modification in inner block */
__attribute__((noinline, optimize("O3")))
int test_nested_unsafe(int x, int y, int z) {
    int val = 0;
    
    if (x > 0) {
        if (y > 0) {
            x = z;  /* MODIFIES outer condition variable */
            val = x + y;
        } else {
            val = x - y;
        }
        global_acc += val;
    } else {
        val = z;
        global_acc += val;
    }
    
    return val;
}

/* ========== HELPER FUNCTIONS ========== */

int external_helper(int val) {
    return val * 3;
}

/* ========== MAIN DRIVER ========== */

int main(int argc, char *argv[]) {
    int i;
    int seed;
    
    /* Use argv or time for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    printf("Testing if-conversion safety check (seed: %d)\n", seed);
    
    /* Initialize test variables with random values */
    int x = rand() % 100;
    int y = rand() % 100;
    int z = rand() % 100;
    float fa = (float)(rand() % 100) / 10.0f;
    float fb = (float)(rand() % 100) / 10.0f;
    volatile int volatile_var = rand() % 200;
    int data[10];
    for (i = 0; i < 10; i++) {
        data[i] = rand() % 50;
    }
    
    /* Execute all test patterns */
    int result1 = test_unsafe_direct_mod(x, y);
    float result2 = test_unsafe_increment(fa, fb);
    int result3 = test_unsafe_pointer(&x, 50);
    int result4 = test_unsafe_volatile(&volatile_var);
    int result5 = test_unsafe_multiple_ifs(x, y, z);
    
    int result6 = test_safe_pattern(x, y);
    float result7 = test_safe_different_var(fa, fb);
    int result8 = test_safe_in_loop(10, data);
    int result9 = test_safe_with_external(x, y);
    
    int result10 = test_mixed_patterns(x, y, z);
    int result11 = test_nested_unsafe(x, y, z);
    
    /* Force use of all results to prevent optimization */
    int final_sum = result1 + (int)result2 + result3 + result4 + result5 +
                    result6 + (int)result7 + result8 + result9 +
                    result10 + result11;
    
    printf("Results: %d (global_acc: %d)\n", final_sum, global_acc);
    printf("All tests completed.\n");
    
    return final_sum > 0 ? 0 : 1;
}
