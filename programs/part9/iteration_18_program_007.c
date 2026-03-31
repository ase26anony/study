/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int *arr, int n, int mode);
extern void analyze_results(int *arr, int n, int threshold);

/* Hot function attribute - will be called many times */
__attribute__((hot)) void hot_function(int *arr, int n, int multiplier) {
    long long sum = 0;
    for (int i = 0; i < n * multiplier; i++) {
        /* Complex branching that varies with input */
        if (i % 3 == 0) {
            sum += arr[i % n] * 2;
        } else if (i % 3 == 1) {
            sum += arr[i % n] / 2;
        } else {
            sum += arr[i % n] + 1;
        }
        
        /* Nested condition for more branch coverage */
        if (sum > 1000000) {
            sum %= 1000000;
        }
    }
    printf("Hot function checksum: %lld\n", sum % 1000);
}

/* Cold function - rarely called */
__attribute__((cold, noinline)) void cold_function(int *arr, int n) {
    if (n <= 0) return;
    
    int min = arr[0], max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > max) max = arr[i];
    }
    printf("Range: %d to %d\n", min, max);
}

/* Medium heat function */
__attribute__((noinline)) void medium_function(int *arr, int n, int seed) {
    srand(seed);
    for (int i = 0; i < n/2; i++) {
        int idx = rand() % n;
        switch (arr[idx] % 4) {
            case 0:
                arr[idx] += 1;
                break;
            case 1:
                arr[idx] -= 1;
                break;
            case 2:
                arr[idx] *= 2;
                break;
            case 3:
                arr[idx] /= 2;
                break;
        }
    }
}

/* Function with complex control flow */
__attribute__((noinline)) void complex_flow(int *arr, int n, int mode) {
    int i = 0;
    while (i < n) {
        if (mode == 1) {
            for (int j = 0; j < 5 && i < n; j++, i++) {
                arr[i] = (arr[i] * 13 + 7) % 97;
            }
        } else if (mode == 2) {
            int k = 0;
            do {
                arr[i] = (arr[i] + i) % 113;
                i++;
                k++;
            } while (k < 3 && i < n);
        } else {
            arr[i] = arr[i] ^ 0xFF;
            i++;
        }
        
        /* Additional branching */
        if (i % 100 == 0 && mode > 0) {
            arr[i-1] = 0;
        }
    }
}

/* Another hot function with loops */
__attribute__((hot, noinline)) void hot_loop_function(int *arr, int n, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < n; i++) {
            /* Branch with different probabilities */
            if (arr[i] > 50) {
                arr[i] -= 10;
            } else if (arr[i] < 20) {
                arr[i] += 5;
            } else {
                arr[i] = (arr[i] * 3) % 100;
            }
        }
    }
}

/* Function called from main with varying parameters */
__attribute__((noinline)) void driver_function(int *arr, int n, int mode, int seed) {
    printf("Driver mode: %d\n", mode);
    
    /* Vary function calls based on mode */
    switch (mode) {
        case 1:
            hot_function(arr, n, 1000);
            medium_function(arr, n, seed);
            cold_function(arr, n);
            break;
        case 2:
            hot_loop_function(arr, n, 500);
            complex_flow(arr, n, 2);
            break;
        case 3:
            for (int i = 0; i < 3; i++) {
                medium_function(arr, n, seed + i);
            }
            hot_function(arr, n, 200);
            break;
        default:
            complex_flow(arr, n, 1);
            hot_loop_function(arr, n, 100);
            cold_function(arr, n);
    }
    
    /* Call external functions */
    process_data(arr, n, mode);
    analyze_results(arr, n, 75);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int seed = 42;
    int array_size = 1000;
    int iterations = 1;
    
    /* Parse command line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iter") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        }
    }
    
    srand(seed);
    
    /* Allocate and initialize array with random data */
    int *data = malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < array_size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Execute multiple iterations for more coverage */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d: ", iter + 1);
        
        /* Vary behavior based on mode and iteration */
        int current_mode = mode + (iter % 3);
        driver_function(data, array_size, current_mode, seed + iter);
        
        /* Occasionally call cold function */
        if (iter % 10 == 0) {
            cold_function(data, array_size);
        }
    }
    
    /* Calculate final checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    printf("Final checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
