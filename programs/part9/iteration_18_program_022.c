/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
int external_func1(int x) __attribute__((noinline));
int external_func2(int x) __attribute__((noinline));

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_use_fullname_test = 0;

/* Hot/Cold function attributes */
void hot_function_1(int iterations) __attribute__((hot, noinline));
void hot_function_2(int iterations) __attribute__((hot, noinline));
void cold_function_1(void) __attribute__((cold, noinline));
void cold_function_2(void) __attribute__((cold, noinline));

/* ==================== FUNCTION DEFINITIONS ==================== */

/* Function with varying execution paths based on mode */
int func_varying_paths(int x, int mode) __attribute__((noinline));
int func_varying_paths(int x, int mode) {
    int result = x;
    
    if (mode == 1) {
        for (int i = 0; i < 10; i++) {
            result += (i % 2 == 0) ? i : -i;
        }
    } else if (mode == 2) {
        int temp = x;
        while (temp > 0) {
            result *= 2;
            temp--;
        }
    } else {
        switch (x % 4) {
            case 0:
                result = x * x;
                break;
            case 1:
                result = x + 100;
                break;
            case 2:
                result = x / 2;
                break;
            default:
                result = x - 50;
                break;
        }
    }
    
    return result;
}

/* Hot function - runs many times */
void hot_function_1(int iterations) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Complex branching to generate varied profile */
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum += i / 2;
        } else {
            sum += (i % 10);
        }
        
        /* Nested loop for more counters */
        for (int j = 0; j < 5; j++) {
            if (j % 2 == 0) {
                sum += j;
            }
        }
    }
    /* Prevent dead code elimination */
    if (sum < 0) printf("Impossible\n");
}

/* Another hot function with different pattern */
void hot_function_2(int iterations) {
    volatile long product = 1;
    int count = 0;
    
    while (count < iterations) {
        if (product > 1000000) {
            product = product / 2;
        } else {
            product = product * 3 + 1;
        }
        
        /* Switch statement for branch coverage */
        switch (count % 5) {
            case 0:
                product += 7;
                break;
            case 1:
                product -= 3;
                break;
            case 2:
                product *= 2;
                break;
            case 3:
                product /= 2;
                break;
            case 4:
                product = product ^ 0xFF;
                break;
        }
        count++;
    }
    
    if (product < 0) printf("Impossible\n");
}

/* Cold functions - rarely called */
void cold_function_1(void) {
    /* Rarely executed code path */
    printf("Cold function 1 executed\n");
    
    /* Dead code that might be eliminated without attributes */
    #ifdef VARIANT_A
    printf("Variant A specific code\n");
    #else
    printf("Standard variant code\n");
    #endif
}

void cold_function_2(void) {
    /* Another rarely called function */
    volatile int x = 0;
    for (int i = 0; i < 3; i++) {
        x += i * i;
    }
    
    if (x > 100) {
        printf("Unexpected\n");
    }
}

/* Function with complex conditional structure */
int complex_decision_tree(int value, int depth) __attribute__((noinline));
int complex_decision_tree(int value, int depth) {
    if (depth <= 0) return value;
    
    int result = value;
    
    if (value % 2 == 0) {
        if (value > 100) {
            result = complex_decision_tree(value / 2, depth - 1);
        } else if (value > 50) {
            result = complex_decision_tree(value * 3 + 1, depth - 1);
        } else {
            for (int i = 0; i < depth; i++) {
                result += (i % 2 == 0) ? i : -i;
            }
        }
    } else {
        switch (value % 3) {
            case 0:
                result = value * value;
                for (int i = 0; i < 3; i++) {
                    result -= i;
                }
                break;
            case 1:
                result = 0;
                for (int i = 0; i < depth && i < 10; i++) {
                    result += complex_decision_tree(i, depth - 1);
                }
                break;
            case 2:
                result = 1;
                int temp = value;
                while (temp > 0) {
                    result *= 2;
                    temp /= 2;
                }
                break;
        }
    }
    
    return result;
}

/* Mathematical computation with varying paths */
double mathematical_series(int terms, int variant) __attribute__((noinline));
double mathematical_series(int terms, int variant) {
    double sum = 0.0;
    
    if (variant == 0) {
        /* Taylor series for sin */
        for (int n = 0; n < terms; n++) {
            double term = 1.0;
            for (int i = 1; i <= 2*n + 1; i++) {
                term *= (2*n + 1.0) / i;
            }
            if (n % 2 == 0) {
                sum += term;
            } else {
                sum -= term;
            }
        }
    } else if (variant == 1) {
        /* Geometric series */
        double r = 0.5;
        for (int n = 0; n < terms; n++) {
            sum += 1.0;
            for (int i = 0; i < n; i++) {
                sum *= r;
            }
        }
    } else {
        /* Fibonacci-like series */
        double a = 1.0, b = 1.0;
        for (int n = 0; n < terms; n++) {
            sum += a;
            double c = a + b;
            a = b;
            b = c;
            if (c > 1000.0) {
                a = 1.0;
                b = 1.0;
            }
        }
    }
    
    return sum;
}

/* ==================== MAIN PROGRAM ==================== */

void parse_arguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            g_use_fullname_test = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    parse_arguments(argc, argv);
    
    /* Seed RNG for variability between runs */
    srand(g_seed);
    
    /* Initialize data array with random values */
    int data_size = 100;
    int data[100];
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Execute hot functions with varying iterations */
    int hot_iterations = g_iterations;
    if (g_mode == 1) {
        hot_iterations = g_iterations * 2;
    } else if (g_mode == 2) {
        hot_iterations = g_iterations / 2;
    }
    
    hot_function_1(hot_iterations);
    hot_function_2(hot_iterations / 2);
    
    /* Execute cold functions conditionally */
    if (g_seed % 10 == 0) {  /* Only 10% of runs */
        cold_function_1();
    }
    if (g_seed % 20 == 0) {  /* Only 5% of runs */
        cold_function_2();
    }
    
    /* Process data with varying algorithms */
    int result = 0;
    for (int i = 0; i < data_size; i++) {
        if (g_algorithm == 0) {
            result += func_varying_paths(data[i], g_mode);
        } else if (g_algorithm == 1) {
            result += complex_decision_tree(data[i], 3);
        } else {
            /* Mix of both */
            if (i % 2 == 0) {
                result += func_varying_paths(data[i], g_mode);
            } else {
                result += complex_decision_tree(data[i], 2);
            }
        }
        
        /* Additional mathematical computation */
        if (i % 7 == 0) {
            double series_sum = mathematical_series(10, g_mode % 3);
            result += (int)series_sum;
        }
    }
    
    /* Call external functions (simulating different object files) */
    result += external_func1(result);
    result += external_func2(result);
    
    /* Final computation that depends on all previous work */
    volatile int final_result = 0;
    for (int i = 0; i < 100; i++) {
        if (result > 0) {
            final_result += (result % (i + 1));
        } else {
            final_result -= ((-result) % (i + 1));
        }
    }
    
    /* Output deterministic result for verification */
    printf("Result: %d (Mode: %d, Seed: %d, Iterations: %d, Algorithm: %d)\n",
           final_result, g_mode, g_seed, g_iterations, g_algorithm);
    
    return 0;
}
