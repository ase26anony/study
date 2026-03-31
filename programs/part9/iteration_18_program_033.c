/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int* data, int size, int mode);
extern void analyze_results(int* data, int size, int threshold);

/* Hot function attribute - will be called many times */
__attribute__((hot)) void hot_function_loop(int iterations) {
    volatile int sum = 0;
    for (int i = 0; i < iterations * 1000; i++) {
        sum += i % 100;
        /* Prevent loop optimization */
        if (sum > 1000000) sum = 0;
    }
}

/* Cold function attribute - rarely called */
__attribute__((cold)) void cold_function(void) {
    printf("[COLD] Diagnostic info\n");
}

/* Prevent inlining to maintain separate function profile */
__attribute__((noinline)) int complex_branching(int x, int y, int mode) {
    int result = 0;
    
    if (mode == 1) {
        if (x > y) {
            result = x * 2;
        } else if (x == y) {
            result = x + y;
        } else {
            result = y / 2;
        }
    } else if (mode == 2) {
        switch (x % 4) {
            case 0: result = y + 1; break;
            case 1: result = y - 1; break;
            case 2: result = y * 2; break;
            case 3: result = y / 2; break;
            default: result = 0;
        }
    } else {
        for (int i = 0; i < x; i++) {
            result += (y >> i) & 1;
        }
    }
    
    return result;
}

__attribute__((noinline)) void process_array(int* arr, int size, int seed) {
    srand(seed);
    
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000;
    }
    
    /* Different sorting paths based on seed */
    if (seed % 3 == 0) {
        /* Bubble sort - many iterations */
        for (int i = 0; i < size - 1; i++) {
            for (int j = 0; j < size - i - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    int temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
    } else if (seed % 3 == 1) {
        /* Selection sort */
        for (int i = 0; i < size - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < size; j++) {
                if (arr[j] < arr[min_idx]) {
                    min_idx = j;
                }
            }
            int temp = arr[min_idx];
            arr[min_idx] = arr[i];
            arr[i] = temp;
        }
    } else {
        /* Insertion sort */
        for (int i = 1; i < size; i++) {
            int key = arr[i];
            int j = i - 1;
            while (j >= 0 && arr[j] > key) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
    }
}

__attribute__((noinline)) int calculate_checksum(int* arr, int size) {
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= arr[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    return checksum;
}

__attribute__((noinline)) void matrix_operation(int size, int mode) {
    /* Create and process matrices with different algorithms */
    int* matrix = malloc(size * size * sizeof(int));
    
    if (!matrix) {
        cold_function();
        return;
    }
    
    /* Fill matrix */
    for (int i = 0; i < size * size; i++) {
        matrix[i] = i % 256;
    }
    
    /* Different processing modes */
    if (mode == 0) {
        /* Transpose */
        for (int i = 0; i < size; i++) {
            for (int j = i + 1; j < size; j++) {
                int temp = matrix[i * size + j];
                matrix[i * size + j] = matrix[j * size + i];
                matrix[j * size + i] = temp;
            }
        }
    } else if (mode == 1) {
        /* Scale */
        for (int i = 0; i < size * size; i++) {
            matrix[i] = (matrix[i] * 2) % 256;
        }
    } else {
        /* Add noise */
        for (int i = 0; i < size * size; i++) {
            matrix[i] = (matrix[i] + (i % 10)) % 256;
        }
    }
    
    free(matrix);
}

int main(int argc, char* argv[]) {
    int seed = 42;
    int array_size = 100;
    int iterations = 10;
    int mode = 0;
    int threshold = 500;
    
    /* Parse command line arguments for different execution paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            threshold = atoi(argv[++i]);
        }
    }
    
    printf("Running with seed=%d, size=%d, iterations=%d, mode=%d\n", 
           seed, array_size, iterations, mode);
    
    /* Initialize random seed for this run */
    srand(seed);
    
    /* Allocate and process data */
    int* data = malloc(array_size * sizeof(int));
    if (!data) {
        return 1;
    }
    
    /* Call functions with different execution profiles */
    process_array(data, array_size, seed);
    
    /* Hot loop - many iterations */
    hot_function_loop(iterations);
    
    /* Complex branching with different modes */
    int branch_result = 0;
    for (int i = 0; i < array_size / 10; i++) {
        branch_result += complex_branching(data[i] % 10, i % 10, mode);
    }
    
    /* Matrix operation - varies with mode */
    matrix_operation(20, mode % 3);
    
    /* External functions (from other object files) */
    process_data(data, array_size, mode);
    analyze_results(data, array_size, threshold);
    
    /* Calculate final checksum */
    int checksum = calculate_checksum(data, array_size);
    
    /* Occasionally call cold function */
    if (seed % 7 == 0) {
        cold_function();
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Branch result: %d\n", branch_result);
    
    free(data);
    return 0;
}
