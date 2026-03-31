/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* __attribute__ directives to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_threshold = 500;
static int g_algorithm = 0;

/* Function 1: Hot path function with high execution count */
HOT NOINLINE
void hot_function_a(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < g_iterations; i++) {
        for (int j = 0; j < size; j++) {
            if (data[j] > g_threshold) {
                sum += data[j] * 2;  /* Hot branch */
            } else {
                sum += data[j] / 2;  /* Cold branch */
            }
        }
    }
    data[0] = sum % 1000;
}

/* Function 2: Medium heat function */
NOINLINE
void medium_function_b(int *data, int size) {
    int temp = 0;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 4) {
            case 0:
                temp += data[i] * 3;
                break;
            case 1:
                temp += data[i] * 2;
                break;
            case 2:
                temp += data[i];
                break;
            case 3:
                temp -= data[i];
                break;
        }
    }
    data[1] = temp;
}

/* Function 3: Cold function with rare execution */
COLD NOINLINE
void cold_function_c(int *data, int size) {
    if (g_mode == 2) {  /* Only executes in mode 2 */
        int count = 0;
        for (int i = 0; i < size; i++) {
            if (data[i] < 0) {
                count++;
            }
        }
        data[2] = count;
    } else {
        data[2] = -1;
    }
}

/* Function 4: Mode-dependent function */
NOINLINE
void mode_dependent_function(int *data, int size) {
    if (g_mode == 0) {
        /* Quick path */
        for (int i = 0; i < size; i += 2) {
            data[i] = data[i] * data[i];
        }
    } else if (g_mode == 1) {
        /* Medium path */
        for (int i = 0; i < size; i++) {
            data[i] = (data[i] * 3) / 2;
        }
    } else {
        /* Complex path */
        for (int i = 0; i < size; i++) {
            int val = data[i];
            if (val % 2 == 0) {
                for (int j = 0; j < 10; j++) {
                    val = (val * 1103515245 + 12345) & 0x7fffffff;
                }
                data[i] = val % 1000;
            }
        }
    }
}

/* Function 5: Algorithm selection function */
NOINLINE
void algorithm_selector(int *data, int size) {
    switch (g_algorithm) {
        case 0:  /* Bubble sort (inefficient, many iterations) */
            for (int i = 0; i < size - 1; i++) {
                for (int j = 0; j < size - i - 1; j++) {
                    if (data[j] > data[j + 1]) {
                        int temp = data[j];
                        data[j] = data[j + 1];
                        data[j + 1] = temp;
                    }
                }
            }
            break;
            
        case 1:  /* Selection sort */
            for (int i = 0; i < size - 1; i++) {
                int min_idx = i;
                for (int j = i + 1; j < size; j++) {
                    if (data[j] < data[min_idx]) {
                        min_idx = j;
                    }
                }
                if (min_idx != i) {
                    int temp = data[i];
                    data[i] = data[min_idx];
                    data[min_idx] = temp;
                }
            }
            break;
            
        case 2:  /* Insertion sort */
            for (int i = 1; i < size; i++) {
                int key = data[i];
                int j = i - 1;
                while (j >= 0 && data[j] > key) {
                    data[j + 1] = data[j];
                    j--;
                }
                data[j + 1] = key;
            }
            break;
            
        default:
            /* Linear pass */
            for (int i = 0; i < size; i++) {
                data[i] = data[i] * 2;
            }
    }
}

/* Function 6: Recursive function for call depth */
NOINLINE
int recursive_function(int n, int depth) {
    if (depth <= 0 || n <= 1) {
        return 1;
    }
    
    if (n % 2 == 0) {
        return recursive_function(n / 2, depth - 1) + 1;
    } else {
        return recursive_function(3 * n + 1, depth - 1) + 1;
    }
}

/* Function 7: Final processing with varied execution */
NOINLINE
void final_processing(int *data, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        /* Complex conditional chain */
        if (data[i] < 100) {
            result += data[i] * 3;
        } else if (data[i] < 500) {
            result += data[i] * 2;
        } else if (data[i] < 800) {
            result += data[i];
        } else {
            result += data[i] / 2;
        }
        
        /* Nested loop with threshold */
        if (g_mode == 1 && data[i] > 700) {
            for (int j = 0; j < 5; j++) {
                result += j;
            }
        }
    }
    data[size - 1] = result;
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--mode N] [--iterations N] [--seed N] "
                   "[--threshold N] [--algorithm N]\n", argv[0]);
            return 0;
        }
    }
    
    /* Initialize random seed */
    srand(g_seed);
    
    /* Create data array */
    const int data_size = 100;
    int *data = (int *)malloc(data_size * sizeof(int));
    
    /* Fill with random data based on mode */
    for (int i = 0; i < data_size; i++) {
        if (g_mode == 0) {
            data[i] = rand() % 200;  /* Small numbers */
        } else if (g_mode == 1) {
            data[i] = rand() % 1000; /* Medium numbers */
        } else {
            data[i] = rand() % 2000; /* Large numbers */
        }
    }
    
    /* Execute functions in different orders based on mode */
    if (g_mode == 0) {
        hot_function_a(data, data_size);
        medium_function_b(data, data_size);
        algorithm_selector(data, data_size);
        final_processing(data, data_size);
    } else if (g_mode == 1) {
        medium_function_b(data, data_size);
        algorithm_selector(data, data_size);
        hot_function_a(data, data_size);
        mode_dependent_function(data, data_size);
        final_processing(data, data_size);
    } else {
        cold_function_c(data, data_size);
        hot_function_a(data, data_size);
        mode_dependent_function(data, data_size);
        algorithm_selector(data, data_size);
        recursive_function(data[0], 10);
        final_processing(data, data_size);
    }
    
    /* Call external functions (from other compilation units) */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Calculate and output checksum */
    int result = checksum(data, data_size);
    printf("Result: %d (Mode: %d, Seed: %d, Iterations: %d)\n", 
           result, g_mode, g_seed, g_iterations);
    
    free(data);
    return 0;
}
