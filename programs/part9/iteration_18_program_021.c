/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* Global configuration */
static int g_verbose = 0;
static int g_seed = 0;
static int g_iterations = 1000;
static int g_algorithm = 0;
static int g_threshold = 50;

/* Function prototypes targeting different coverage scenarios */
static void __attribute__((noinline)) process_mode_a(int *data, int size);
static void __attribute__((noinline)) process_mode_b(int *data, int size);
static void __attribute__((hot)) hot_loop_function(int *data, int size);
static void __attribute__((cold)) cold_function(int *data, int size);
static void __attribute__((noinline)) complex_branching(int value);
static void __attribute__((noinline)) nested_conditions(int a, int b, int c);
static int __attribute__((noinline)) switch_based_processing(int x);

/* Helper functions with different execution profiles */
static void fill_random_data(int *data, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
}

static void bubble_sort(int *data, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                int temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

static void quick_sort_partial(int *data, int left, int right, int depth) {
    if (left >= right || depth <= 0) return;
    
    int pivot = data[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        while (data[i] < pivot) i++;
        while (data[j] > pivot) j--;
        if (i <= j) {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;
            i++;
            j--;
        }
    }
    
    if (left < j) quick_sort_partial(data, left, j, depth - 1);
    if (i < right) quick_sort_partial(data, i, right, depth - 1);
}

/* Hot function - runs many times */
static void __attribute__((hot)) hot_loop_function(int *data, int size) {
    long long sum = 0;
    for (int iter = 0; iter < g_iterations; iter++) {
        for (int i = 0; i < size; i++) {
            /* Complex branching inside hot loop */
            if (data[i] > g_threshold) {
                sum += data[i] * 2;
                if (data[i] > g_threshold * 2) {
                    sum -= data[i] / 2;
                }
            } else if (data[i] < -g_threshold) {
                sum += data[i] / 3;
            } else {
                sum += data[i];
            }
            
            /* Nested conditions */
            if (i % 3 == 0) {
                if (data[i] % 2 == 0) {
                    sum += 1;
                } else {
                    sum -= 1;
                }
            }
        }
    }
    
    if (g_verbose) {
        printf("Hot function processed %lld units\n", sum);
    }
}

/* Cold function - runs few times */
static void __attribute__((cold)) cold_function(int *data, int size) {
    if (size <= 1) return;
    
    int min = data[0];
    int max = data[0];
    
    for (int i = 1; i < size; i++) {
        if (data[i] < min) {
            min = data[i];
        } else if (data[i] > max) {
            max = data[i];
        }
        
        /* Rarely executed branch */
        if (data[i] == 777 && g_algorithm == 2) {
            printf("Lucky number found!\n");
        }
    }
    
    if (g_verbose) {
        printf("Range: %d to %d\n", min, max);
    }
}

/* Complex branching function */
static void __attribute__((noinline)) complex_branching(int value) {
    if (value < 0) {
        if (value < -100) {
            /* Rare negative branch */
            if (g_verbose) printf("Very negative\n");
        } else {
            if (g_verbose) printf("Negative\n");
        }
    } else if (value == 0) {
        if (g_verbose) printf("Zero\n");
    } else if (value < 100) {
        if (value < 50) {
            if (g_verbose) printf("Small positive\n");
        } else {
            if (g_verbose) printf("Medium positive\n");
        }
    } else {
        if (value > 1000) {
            if (g_verbose) printf("Very large\n");
        } else {
            if (g_verbose) printf("Large\n");
        }
    }
}

/* Nested conditions function */
static void __attribute__((noinline)) nested_conditions(int a, int b, int c) {
    if (a > 0) {
        if (b > 0) {
            if (c > 0) {
                if (g_verbose) printf("All positive\n");
            } else if (c < 0) {
                if (g_verbose) printf("a,b positive, c negative\n");
            } else {
                if (g_verbose) printf("a,b positive, c zero\n");
            }
        } else if (b < 0) {
            if (c != 0) {
                if (g_verbose) printf("a positive, b negative, c non-zero\n");
            }
        }
    } else if (a < 0) {
        if (b == 0) {
            if (g_verbose) printf("a negative, b zero\n");
        }
    }
}

/* Switch-based processing */
static int __attribute__((noinline)) switch_based_processing(int x) {
    int result = 0;
    
    switch (x % 7) {
        case 0:
            result = x * 2;
            break;
        case 1:
            result = x + 10;
            if (x > 100) result += 5;
            break;
        case 2:
            result = x - 5;
            /* Fall through */
        case 3:
            result += 3;
            break;
        case 4:
            if (x < 0) {
                result = -x;
            } else {
                result = x / 2;
            }
            break;
        case 5:
            result = x * x;
            break;
        case 6:
        default:
            result = 1;
            break;
    }
    
    return result;
}

/* Mode-specific processing */
static void __attribute__((noinline)) process_mode_a(int *data, int size) {
    /* Algorithm A: Bubble sort + hot loop */
    bubble_sort(data, size);
    hot_loop_function(data, size);
    
    for (int i = 0; i < size; i++) {
        complex_branching(data[i]);
        data[i] = switch_based_processing(data[i]);
    }
}

static void __attribute__((noinline)) process_mode_b(int *data, int size) {
    /* Algorithm B: Quick sort partial + cold function */
    quick_sort_partial(data, 0, size - 1, 3);
    cold_function(data, size);
    
    for (int i = 0; i < size; i += 2) {
        nested_conditions(data[i], data[i + 1], i);
        if (i % 3 == 0) {
            data[i] = data[i] * 3 / 2;
        }
    }
}

/* Main execution logic */
int main(int argc, char *argv[]) {
    /* Parse command line arguments to create varied profiles */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] [--threshold T] [--verbose]\n", argv[0]);
            return 0;
        }
    }
    
    const int data_size = 100;
    int *data = malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with seed-dependent values */
    fill_random_data(data, data_size, g_seed);
    
    /* Execute different code paths based on algorithm */
    switch (g_algorithm) {
        case 0:
            /* Default: mix of hot and cold */
            hot_loop_function(data, data_size);
            cold_function(data, data_size);
            for (int i = 0; i < data_size; i++) {
                complex_branching(data[i]);
            }
            break;
            
        case 1:
            process_mode_a(data, data_size);
            break;
            
        case 2:
            process_mode_b(data, data_size);
            break;
            
        case 3:
            /* Alternate between hot and cold */
            for (int i = 0; i < 10; i++) {
                if (i % 3 == 0) {
                    hot_loop_function(data, 10);
                } else {
                    cold_function(data, 10);
                }
            }
            break;
            
        default:
            /* Execute all functions */
            hot_loop_function(data, data_size);
            cold_function(data, data_size);
            process_mode_a(data, data_size);
            process_mode_b(data, data_size);
            break;
    }
    
    /* Call external functions (from other object files) */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Calculate and output deterministic result */
    int result = checksum(data, data_size);
    printf("Result checksum: %d\n", result);
    
    free(data);
    return 0;
}
