/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int* data, int size, int threshold);
extern int validate_result(long checksum, int mode);

/* Hot function attribute - will be called many times */
__attribute__((hot)) void hot_function_1(int iterations) {
    volatile int sum = 0;
    for (int i = 0; i < iterations * 1000; i++) {
        sum += i % 17;  /* Prevent optimization */
    }
}

/* Cold function attribute - rarely called */
__attribute__((cold)) void cold_function_1(void) {
    printf("Cold function executed\n");
}

/* Function with complex branching - different paths per run */
__attribute__((noinline)) int complex_branching(int value, int mode) {
    int result = 0;
    
    if (mode == 1) {
        if (value > 1000) {
            result = value * 2;
        } else if (value > 500) {
            result = value + 100;
        } else {
            result = value / 2;
        }
    } else if (mode == 2) {
        switch (value % 5) {
            case 0: result = value << 1; break;
            case 1: result = value >> 1; break;
            case 2: result = value + 777; break;
            case 3: result = value - 333; break;
            default: result = value * 3; break;
        }
    } else {
        /* Nested conditionals for more coverage */
        if (value % 3 == 0) {
            if (value % 7 == 0) {
                result = value * 7;
            } else {
                result = value * 3;
            }
        } else {
            result = value + 1;
        }
    }
    
    return result;
}

/* Another hot function with loop */
__attribute__((hot, noinline)) void hot_function_2(int* array, int size) {
    #pragma GCC unroll 0  /* Prevent loop unrolling */
    for (int i = 0; i < size; i++) {
        if (array[i] > 0) {
            array[i] = complex_branching(array[i], i % 3);
        } else {
            array[i] = -array[i];
        }
    }
}

/* Function with varying execution count */
__attribute__((noinline)) void variable_workload(int base, int multiplier) {
    int limit = base * multiplier;
    volatile int counter = 0;
    
    for (int i = 0; i < limit; i++) {
        counter += (i % 19) * (i % 23);
        if (i % 1000 == 0) {
            cold_function_1();  /* Rare call to cold function */
        }
    }
}

/* Main processing function */
long process_mode(int mode, int seed, int iterations) {
    srand(seed);
    long total_checksum = 0;
    
    /* Create data with different distributions per mode */
    int data_size = 10000;
    int* data = malloc(data_size * sizeof(int));
    
    for (int i = 0; i < data_size; i++) {
        if (mode == 1) {
            data[i] = rand() % 2000;  /* Uniform distribution */
        } else if (mode == 2) {
            data[i] = (rand() % 100) * (rand() % 100);  /* Skewed distribution */
        } else {
            data[i] = rand() % 5000 - 2500;  /* Includes negative values */
        }
    }
    
    /* Execute hot functions many times */
    for (int i = 0; i < iterations; i++) {
        hot_function_1(i + 1);
        
        if (i % 10 == 0) {
            hot_function_2(data, data_size / 100);
        }
    }
    
    /* Variable workload based on mode */
    variable_workload(mode * 100, iterations);
    
    /* Process data with external function */
    process_data(data, data_size, mode * 500);
    
    /* Calculate checksum */
    for (int i = 0; i < data_size; i++) {
        total_checksum += data[i];
    }
    
    free(data);
    return total_checksum;
}

int main(int argc, char** argv) {
    int mode = 1;
    int seed = 42;
    int iterations = 100;
    
    /* Parse command line arguments for different execution paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--mode N] [--seed N] [--iterations N]\n", argv[0]);
            printf("  --mode 1-3: Different processing algorithms\n");
            printf("  --seed N: Random seed for reproducible runs\n");
            printf("  --iterations N: Number of hot loop iterations\n");
            return 0;
        }
    }
    
    printf("Running mode %d with seed %d, iterations %d\n", mode, seed, iterations);
    
    long checksum = process_mode(mode, seed, iterations);
    
    /* Validate result with external function */
    int valid = validate_result(checksum, mode);
    
    printf("Checksum: %ld, Valid: %d\n", checksum, valid);
    
    return 0;
}
