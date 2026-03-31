/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* Configuration from command line */
static int g_seed = 0;
static int g_iterations = 1000;
static int g_algorithm = 0;
static int g_verbose = 0;
static int g_use_full_workload = 0;

/* __attribute__((noinline)) ensures functions aren't inlined */
__attribute__((noinline)) 
static void parse_arguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--full-workload") == 0) {
            g_use_full_workload = 1;
        }
    }
}

/* Hot function - runs many times */
__attribute__((hot))
__attribute__((noinline))
static void bubble_sort(int *arr, int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Cold function - runs rarely */
__attribute__((cold))
__attribute__((noinline))
static void validate_array(const int *arr, int n) {
    if (n <= 1) return;
    
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            if (g_verbose) {
                fprintf(stderr, "Validation failed at index %d\n", i);
            }
            break;
        }
    }
}

/* Function with complex branching */
__attribute__((noinline))
static int process_with_algorithm(int *data, int size, int algo) {
    int result = 0;
    
    switch (algo) {
        case 0: /* Default algorithm - always runs */
            bubble_sort(data, size);
            for (int i = 0; i < size; i++) {
                if (data[i] % 2 == 0) {
                    result += data[i] * 2;
                } else {
                    result += data[i] / 2;
                }
            }
            break;
            
        case 1: /* Alternative algorithm */
            for (int i = 0; i < size; i++) {
                if (i % 3 == 0) {
                    data[i] *= 3;
                } else if (i % 3 == 1) {
                    data[i] /= 3;
                } else {
                    data[i] += 3;
                }
                result += data[i];
            }
            break;
            
        case 2: /* Another variant */
            for (int i = 0; i < size; i++) {
                int val = data[i];
                if (val > 1000) val = 1000;
                if (val < -1000) val = -1000;
                
                while (val > 0) {
                    result++;
                    val--;
                }
                while (val < 0) {
                    result--;
                    val++;
                }
            }
            break;
            
        default:
            result = -1;
            if (g_verbose) {
                fprintf(stderr, "Unknown algorithm: %d\n", algo);
            }
    }
    
    return result;
}

/* Function called in different patterns */
__attribute__((noinline))
static void process_iteration(int iteration, int *data, int size) {
    if (iteration % 100 == 0) {
        validate_array(data, size);
    }
    
    if (iteration % 50 == 0) {
        int temp[size];
        memcpy(temp, data, size * sizeof(int));
        bubble_sort(temp, size);
    }
    
    int mod = iteration % 10;
    if (mod == 0 || mod == 3 || mod == 7) {
        data[iteration % size] = rand() % 10000;
    }
}

/* Main workload */
__attribute__((noinline))
static int run_workload(int algorithm, int iterations, int data_size) {
    int *data = malloc(data_size * sizeof(int));
    if (!data) return -1;
    
    /* Initialize with random data */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000 - 1000;
    }
    
    int total = 0;
    
    /* Hot loop - runs many times */
    for (int i = 0; i < iterations; i++) {
        process_iteration(i, data, data_size);
        
        if (i % 100 == 0) {
            int *copy = malloc(data_size * sizeof(int));
            if (copy) {
                memcpy(copy, data, data_size * sizeof(int));
                total += process_with_algorithm(copy, data_size, algorithm);
                free(copy);
            }
        }
    }
    
    /* Final processing */
    if (g_use_full_workload) {
        total += process_with_algorithm(data, data_size, algorithm);
        process_data_hot(data, data_size, 500);
    } else {
        process_data_cold(data, data_size);
    }
    
    int final_checksum = checksum(data, data_size);
    
    free(data);
    return total + final_checksum;
}

int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);
    
    if (g_seed == 0) {
        g_seed = time(NULL);
    }
    srand(g_seed);
    
    if (g_verbose) {
        printf("Running with seed=%d, iterations=%d, algorithm=%d\n",
               g_seed, g_iterations, g_algorithm);
    }
    
    int data_size = 100;
    if (g_iterations > 10000) {
        data_size = 500;
    }
    
    int result = run_workload(g_algorithm, g_iterations, data_size);
    
    printf("Result: %d\n", result);
    return 0;
}
